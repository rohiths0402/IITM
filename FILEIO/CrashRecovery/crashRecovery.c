#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <aio.h>

#define DB_FILE "database.db"
#define WAL_FILE "database.wal"
#define DB_SIZE 4096
#define MAX_RECORDS 100
#define RECORD_SIZE 40

typedef struct {
    int id;
    char name[32];
    int active;
} record_t;

// WAL entry structure
typedef struct {
    int record_id;
    record_t data;
    long timestamp;
    char checksum[16];
} wal_entry_t;

// Database context
typedef struct {
    int db_fd;
    int wal_fd;
    record_t *db_mmap;
    size_t db_size;
    int next_id;
} db_context_t;

// Simple checksum calculation
void calculate_checksum(const record_t *record, char *checksum) {
    unsigned int hash = 0;
    const char *data = (const char *)record;
    for (size_t i = 0; i < sizeof(record_t); i++) {
        hash = hash * 31 + data[i];
    }
    snprintf(checksum, 16, "%08X", hash);
}

// Initialize database files
int init_database(db_context_t *ctx) {
    struct stat st;
    
    // Open/create database file
    ctx->db_fd = open(DB_FILE, O_RDWR | O_CREAT, 0644);
    if (ctx->db_fd == -1) {
        perror("open database file");
        return -1;
    }
    
    // Check if file exists and has content
    if (fstat(ctx->db_fd, &st) == -1) {
        perror("fstat database");
        return -1;
    }
    
    // If file is empty, initialize it
    if (st.st_size == 0) {
        if (ftruncate(ctx->db_fd, DB_SIZE) == -1) {
            perror("ftruncate database");
            return -1;
        }
        printf("✓ Initialized new database file (%d bytes)\n", DB_SIZE);
    }
    
    // Memory map the database file
    ctx->db_mmap = mmap(NULL, DB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->db_fd, 0);
    if (ctx->db_mmap == MAP_FAILED) {
        perror("mmap database");
        return -1;
    }
    
    ctx->db_size = DB_SIZE;
    
    // Open WAL file
    ctx->wal_fd = open(WAL_FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (ctx->wal_fd == -1) {
        perror("open WAL file");
        return -1;
    }
    
    // Find next available ID
    ctx->next_id = 1;
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (ctx->db_mmap[i].active && ctx->db_mmap[i].id >= ctx->next_id) {
            ctx->next_id = ctx->db_mmap[i].id + 1;
        }
    }
    
    printf("✓ Database initialized (next ID: %d)\n", ctx->next_id);
    return 0;
}

// Write entry to WAL with fsync
int write_wal_entry(db_context_t *ctx, const record_t *record) {
    wal_entry_t entry;
    entry.record_id = record->id;
    entry.data = *record;
    entry.timestamp = time(NULL);
    calculate_checksum(record, entry.checksum);
    
    ssize_t written = write(ctx->wal_fd, &entry, sizeof(entry));
    if (written != sizeof(entry)) {
        perror("write WAL entry");
        return -1;
    }
    
    // Critical: fsync to ensure WAL entry is on disk
    if (fsync(ctx->wal_fd) == -1) {
        perror("fsync WAL");
        return -1;
    }
    
    printf("✓ WAL entry written and synced (ID: %d, checksum: %s)\n", 
           record->id, entry.checksum);
    return 0;
}

// Apply record to memory-mapped database
int apply_to_database(db_context_t *ctx, const record_t *record) {
    // Find slot for this record
    int slot = -1;
    
    // First, look for existing record with same ID
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (ctx->db_mmap[i].active && ctx->db_mmap[i].id == record->id) {
            slot = i;
            break;
        }
    }
    
    // If not found, find empty slot
    if (slot == -1) {
        for (int i = 0; i < MAX_RECORDS; i++) {
            if (!ctx->db_mmap[i].active) {
                slot = i;
                break;
            }
        }
    }
    
    if (slot == -1) {
        printf("✗ Database full!\n");
        return -1;
    }
    
    // Apply the change
    ctx->db_mmap[slot] = *record;
    
    // Sync the memory-mapped region to disk
    if (msync(ctx->db_mmap, ctx->db_size, MS_SYNC) == -1) {
        perror("msync database");
        return -1;
    }
    
    printf("✓ Applied to database slot %d (ID: %d)\n", slot, record->id);
    return 0;
}

// Insert a new record (with WAL)
int insert_record(db_context_t *ctx, const char *name) {
    record_t record;
    record.id = ctx->next_id++;
    strncpy(record.name, name, sizeof(record.name) - 1);
    record.name[sizeof(record.name) - 1] = '\0';
    record.active = 1;
    
    printf("\n🔄 Inserting record: ID=%d, name='%s'\n", record.id, record.name);
    
    // Step 1: Write to WAL first (with fsync)
    if (write_wal_entry(ctx, &record) == -1) {
        return -1;
    }
    
    // Step 2: Apply to database (this could be interrupted by crash)
    if (apply_to_database(ctx, &record) == -1) {
        return -1;
    }
    
    return record.id;
}

// Simulate crash at specified point
void simulate_crash(const char *crash_point) {
    printf("\n💥 SIMULATING CRASH AT: %s\n", crash_point);
    printf("💥 Process terminating unexpectedly...\n");
    exit(42);  // Unexpected exit code
}

// Recovery: replay WAL entries
int recover_from_wal(db_context_t *ctx) {
    struct stat st;
    if (fstat(ctx->wal_fd, &st) == -1) {
        perror("fstat WAL");
        return -1;
    }
    
    if (st.st_size == 0) {
        printf("✓ No WAL entries to recover\n");
        return 0;
    }
    
    printf("\n🔄 Starting WAL recovery...\n");
    printf("📄 WAL file size: %ld bytes\n", st.st_size);
    
    // Read WAL from beginning
    lseek(ctx->wal_fd, 0, SEEK_SET);
    
    wal_entry_t entry;
    int entries_recovered = 0;
    
    while (read(ctx->wal_fd, &entry, sizeof(entry)) == sizeof(entry)) {
        // Verify checksum
        char expected_checksum[16];
        calculate_checksum(&entry.data, expected_checksum);
        
        if (strcmp(entry.checksum, expected_checksum) != 0) {
            printf("✗ WAL entry corrupted (checksum mismatch)\n");
            continue;
        }
        
        // Check if this entry is already applied
        int already_applied = 0;
        for (int i = 0; i < MAX_RECORDS; i++) {
            if (ctx->db_mmap[i].active && 
                ctx->db_mmap[i].id == entry.data.id &&
                strcmp(ctx->db_mmap[i].name, entry.data.name) == 0) {
                already_applied = 1;
                break;
            }
        }
        
        if (!already_applied) {
            printf("🔄 Replaying WAL entry: ID=%d, name='%s'\n", 
                   entry.data.id, entry.data.name);
            
            if (apply_to_database(ctx, &entry.data) == -1) {
                printf("✗ Failed to apply WAL entry\n");
                continue;
            }
            entries_recovered++;
        } else {
            printf("⏭️  Skipping already applied entry: ID=%d\n", entry.data.id);
        }
    }
    
    printf("✓ Recovery complete: %d entries replayed\n", entries_recovered);
    
    // Optionally truncate WAL after successful recovery
    if (entries_recovered > 0) {
        if (ftruncate(ctx->wal_fd, 0) == -1) {
            perror("truncate WAL");
        } else {
            printf("✓ WAL file cleared after successful recovery\n");
        }
    }
    
    return entries_recovered;
}

// Display database contents
void display_database(db_context_t *ctx) {
    printf("\n📊 Current Database State:\n");
    printf("┌─────┬──────────────────────────────────┬────────┐\n");
    printf("│ ID  │ Name                             │ Active │\n");
    printf("├─────┼──────────────────────────────────┼────────┤\n");
    
    int count = 0;
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (ctx->db_mmap[i].active) {
            printf("│ %-3d │ %-32s │   ✓    │\n", 
                   ctx->db_mmap[i].id, ctx->db_mmap[i].name);
            count++;
        }
    }
    
    if (count == 0) {
        printf("│                   (empty database)                    │\n");
    }
    
    printf("└─────┴──────────────────────────────────┴────────┘\n");
    printf("Total records: %d\n", count);
}

// Cleanup resources
void cleanup(db_context_t *ctx) {
    if (ctx->db_mmap && ctx->db_mmap != MAP_FAILED) {
        munmap(ctx->db_mmap, ctx->db_size);
    }
    if (ctx->db_fd >= 0) {
        close(ctx->db_fd);
    }
    if (ctx->wal_fd >= 0) {
        close(ctx->wal_fd);
    }
}

// Main demonstration
int main(int argc, char *argv[]) {
    db_context_t ctx = {0};
    
    printf("🗄️  WAL Database Crash Recovery Simulation\n");
    printf("==========================================\n");
    
    // Initialize database
    if (init_database(&ctx) == -1) {
        cleanup(&ctx);
        return 1;
    }
    
    // Perform recovery on startup
    recover_from_wal(&ctx);
    
    display_database(&ctx);
    
    if (argc > 1) {
        // Command-line mode for specific operations
        if (strcmp(argv[1], "insert") == 0 && argc > 2) {
            insert_record(&ctx, argv[2]);
            display_database(&ctx);
        } else if (strcmp(argv[1], "crash-after-wal") == 0 && argc > 2) {
            printf("\n🎯 Crash simulation mode: after WAL write\n");
            
            record_t record;
            record.id = ctx.next_id++;
            strncpy(record.name, argv[2], sizeof(record.name) - 1);
            record.name[sizeof(record.name) - 1] = '\0';
            record.active = 1;
            
            // Write to WAL
            write_wal_entry(&ctx, &record);
            
            // Simulate crash before applying to database
            simulate_crash("after WAL write, before database update");
        }
    } else {
        // Interactive demonstration
        printf("\n🔄 Running demonstration sequence...\n");
        
        // Insert some records normally
        insert_record(&ctx, "Alice");
        insert_record(&ctx, "Bob");
        display_database(&ctx);
        
        printf("\n🎯 Next: Run './program crash-after-wal Charlie' to simulate crash\n");
        printf("🎯 Then: Run './program' again to see recovery in action\n");
    }
    
    cleanup(&ctx);
    return 0;
}