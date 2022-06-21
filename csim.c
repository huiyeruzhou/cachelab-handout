#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cachelab.h"
typedef struct
{
    int *valid;
    int *line_tags;
    int access_record[10];
    int record_head;
    int record_tail;
} cache_set;

void Printhelp(void);

int main(int argc, char **argv)
{
    // if no arg, print usage
    if (argc == 1)
    {
        Printhelp();
        return 0;
    }

    // Prase args
    int flag_v = 0,
        flag_s = 0,
        flag_E = 0,
        flag_b = 0,
        flag_t = 0;
    char t[256];
    int s, E, b;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            Printhelp();
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            flag_v = 1;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            if (flag_s != 0)
            {
                printf("Error: multiple -s\n");
                Printhelp();
                return 0;
            }

            flag_s = 1;
            // if matched ,read next arg for value
            i++;
            if (i >= argc)
            {
                printf("option requires an argument -- 's'\n");
                Printhelp();
                return 0;
            }
            else
            {
                if (1 != sscanf(argv[i], "%d", &s))
                {
                    printf("option requires an argument -- 's'\n");
                    Printhelp();
                    return 0;
                }
            }
        }
        else if (strcmp(argv[i], "-E") == 0)
        {
            if (flag_E != 0)
            {
                printf("Error: multiple -E\n");
                Printhelp();
                return 0;
            }
            flag_E = 1;

            // if matched ,read next arg for value
            i++;
            if (i >= argc)
            {
                printf("option requires an argument -- 'E'\n");
                Printhelp();
                return 0;
            }
            else
            {
                if (1 != sscanf(argv[i], "%d", &E))
                {
                    printf("option requires an argument -- 'E'\n");
                    Printhelp();
                    return 0;
                }
            }
        }
        else if (strcmp(argv[i], "-b") == 0)
        {
            if (flag_b != 0)
            {
                printf("Error: multiple -b\n");
                Printhelp();
                return 0;
            }
            flag_b = 1;

            // if matched ,read next arg for value
            i++;
            if (i >= argc)
            {
                printf("option requires an argument -- 'b'\n");
                Printhelp();
                return 0;
            }
            else
            {
                if (1 != sscanf(argv[i], "%d", &b))
                {
                    printf("option requires an argument -- 'b'\n");
                    Printhelp();
                    return 0;
                }
            }
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            if (flag_t != 0)
            {
                printf("Error: multiple -t\n");
                Printhelp();
                return 0;
            }
            flag_t = 1;

            // if matched ,read next arg for value
            i++;
            if (i >= argc)
            {
                printf("option requires an argument -- 't'\n");
                Printhelp();
                return 0;
            }
            else
            {
                if (1 != sscanf(argv[i], "%255s", t))
                {
                    printf("option requires an argument -- 't'\n");
                    Printhelp();
                    return 0;
                }
            }
        }
        else
        {
            printf("Invalid option %256s\n", argv[i]);
            Printhelp();
            return 0;
        }
    }
    if (!flag_b || !flag_E || !flag_s || !flag_t)
    {
        printf("Missing required command line argument\n");
        printf("%d,%d,%d,%s", s, E, b, t);
        Printhelp();
        return 0;
    }
    
    // init cache
    cache_set *cache = (cache_set *)malloc(sizeof(cache_set) * (1 << s));
    for (int i = 0; i < (1 << s); i++)
    {
        cache[i].valid = (int *)malloc(sizeof(int) * E);
        cache[i].line_tags = (int *)malloc(sizeof(int) * E);
        for (int j = 0; j < E; j++)
        {
            cache[i].valid[j] = 0;
            cache[i].record_head = 0;
            cache[i].record_tail = 0;
        }
    }

    // read input
    FILE *trace = fopen(t, "r");
    if (trace == NULL)
    {
        printf("Error: cannot open file %s\n", t);
        return 0;
    }

    // simulate memory reference
    int hit_count = 0, miss_count = 0, eviction_count = 0;
    char buf[256];
    while (fgets(buf, 255, trace) != NULL)
    {
        // skip instruction reference
        if (buf[0] != ' ')
            continue;

        // print for verbose
        if (flag_v)
        {
            buf[strlen(buf) - 1] = '\0';
            printf("%s ", buf);
        }
        // parse reference
        int address;
        char type;
        sscanf(buf + 1, "%c %x", &type, &address);

        // parse address
        int set_mask = (1 << s) - 1;
        int set_index = (address >> b) & set_mask;
        int tag = address >> (b + s);

        // check for hit
        int hit_flag = 0;
        for (int i = 0; i < E; i++)
        {
            if (cache[set_index].valid[i] == 1 && cache[set_index].line_tags[i] == tag)
            {
                hit_flag = 1;
                cache[set_index].access_record[cache[set_index].record_tail] = tag;
                cache[set_index].record_tail = (cache[set_index].record_tail + 1) % 10;
                if (cache[set_index].record_tail == cache[set_index].record_head)
                {
                    cache[set_index].record_head = (cache[set_index].record_head + 1) % 10;
                }
                if (type == 'M')
                {
                    cache[set_index].access_record[cache[set_index].record_tail] = tag;
                    cache[set_index].record_tail = (cache[set_index].record_tail + 1) % 10;
                    if (cache[set_index].record_tail == cache[set_index].record_head)
                    {
                        cache[set_index].record_head = (cache[set_index].record_head + 1) % 10;
                    }
                }
                break;
            }
        }

        // calculate
        if (hit_flag)
        {
            if (type == 'M')
            {
                hit_count += 2;
                if (flag_v)
                {
                    printf("hit hit\n");
                }
            }
            else
            {
                hit_count += 1;
                if (flag_v)
                {
                    printf("hit\n");
                }
            }
        }
        else
        {
            // calculate miss_count
            if (type == 'M')
            {
                miss_count += 1;
                hit_count += 1;
            }
            else
            {
                miss_count += 1;
            }
            if (flag_v)
            {
                printf("miss");
            }

            // check for cold conflict
            int cold_flag = 0;
            for (int i = 0; i < E; i++)
            {
                if (cache[set_index].valid[i] == 0)
                {
                    cache[set_index].valid[i] = 1;
                    cache[set_index].line_tags[i] = tag;
                    cache[set_index].access_record[cache[set_index].record_tail] = tag;
                    cache[set_index].record_tail = (cache[set_index].record_tail + 1) % 10;
                    if (cache[set_index].record_tail == cache[set_index].record_head)
                    {
                        cache[set_index].record_head = (cache[set_index].record_head + 1) % 10;
                    }

                    cold_flag = 1;
                    if (flag_v)
                    {
                        if (type == 'M')
                        {
                            printf(" hit\n");
                        }
                        else
                        {
                            printf("\n");
                        }
                    }
                    break;
                }
            }
            if (cold_flag)
                continue;

            // eviction
            int *access_cnt = (int *)malloc(sizeof(int) * E);
            for (int i = 0; i < E; i++)
                access_cnt[i] = 0;

            for (int i = 0; i < E; i++)
            {
                for (int j = cache[set_index].record_head; j % 10 != cache[set_index].record_tail % 10; j++)
                {
                    if (cache[set_index].access_record[j] == cache[set_index].line_tags[i])
                    {
                        access_cnt[i] += 1;
                    }
                }
            }
            int least = 0;
            for (int i = 0; i < E; i++)
            {
                if (access_cnt[i] < access_cnt[least])
                    least = i;
            }
            eviction_count += 1;

            cache[set_index].line_tags[least] = tag;
            cache[set_index].access_record[cache[set_index].record_tail] = tag;
            cache[set_index].record_tail = (cache[set_index].record_tail + 1) % 10;
            if (cache[set_index].record_tail == cache[set_index].record_head)
            {
                cache[set_index].record_head = (cache[set_index].record_head + 1) % 10;
            }

            if (type == 'M')
            {
                cache[set_index].access_record[cache[set_index].record_tail] = tag;
                cache[set_index].record_tail = (cache[set_index].record_tail + 1) % 10;
                if (cache[set_index].record_tail == cache[set_index].record_head)
                {
                    cache[set_index].record_head = (cache[set_index].record_head + 1) % 10;
                }
            }
            if (flag_v)
            {
                if (type == 'M')
                    printf(" eviction hit\n");
                else
                    printf(" eviction\n");
            }
        }
    }

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
void Printhelp(void)
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h         Print this help message.\n"
           "  -v         Optional verbose flag.\n"
           "  -s <num>   Number of set index bits.\n"
           "  -E <num>   Number of lines per set.\n"
           "  -b <num>   Number of block offset bits.\n"
           "  -t <file>  Trace file.\n"
           "\n"
           "Examples:\n"
           "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
