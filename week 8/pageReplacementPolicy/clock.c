#include <stdio.h>
 
#define MAX_FRAMES 3
#define MAX_PAGES 12
 
int pages[MAX_PAGES] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
 
void simulateClock()
{
    int buffer[MAX_FRAMES];
    int refBit[MAX_FRAMES];
    int pointer = 0, hits = 0, misses = 0;
 
    for (int i = 0; i < MAX_FRAMES; i++)
    {
        buffer[i] = -1;
        refBit[i] = 0;
    }
 
    printf("Clock: \n");
 
    for (int i = 0; i < MAX_PAGES; i++)
    {
        int page = pages[i];
        int found = 0;
 
        for (int j = 0; j < MAX_PAGES; j++)
        {
            if (buffer[j] == page)
            {
                found = 1;
                refBit[j] = 1;
                hits++;
                break;
            }
        }
 
        if (!found)
        {
            misses++;
            while (1)
            {
                if (refBit[pointer] == 0)
                {
                    buffer[pointer] = page;
                    refBit[pointer] = 1;
                    pointer = (pointer + 1) % MAX_FRAMES;
                    break;
                }
                else
                {
                    refBit[pointer] = 0;
                    pointer = (pointer + 1) % MAX_FRAMES;
                }
            }
        }
 
        printf("Request %2d -> ", page);
        for (int j = 0; j < MAX_FRAMES; j++)
            if (buffer[j] != -1)
                printf("%d ", buffer[j]);
 
        printf("\n");
    }
 
    printf("\nTotal Hits: %d\nTotal Misses: %d\nHit Ratio: %.2f\n",
           hits, misses, (float)hits / (hits + misses));
}
 
int main()
{
    simulateClock();
    return 0;
}