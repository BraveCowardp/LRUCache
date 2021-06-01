#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define VALID 0
#define TAG 1
#define LRUCOUNTER 2

#define HIT 0
#define MISS 1
#define EVICTION 2

FILE *file;
int Hits=0,Misses=0,Evictions=0,helpflag=0;
_Bool verbose=0;
void print_help()
{
    printf("help\n");
}

struct cache
{
    /* data */
    unsigned long s,E,b,S;
    unsigned long **cache;
}Cache;

void creatCache()
{
    Cache.S=1<<Cache.s;
    Cache.cache=(unsigned long**)malloc(Cache.E*Cache.S*sizeof(unsigned long*));
    for(int i=0;i<Cache.E*Cache.S;i++)
    {
        Cache.cache[i]=(unsigned long*)malloc(3*sizeof(unsigned long));
        Cache.cache[i][VALID]=0;
        Cache.cache[i][TAG]=0;
        Cache.cache[i][LRUCOUNTER]=0;
    }
}




void freeCache()
{
    for(int i=0;i<Cache.E*Cache.S;i++)
    {
        free(Cache.cache[i]);

    }
    free(Cache.cache);
}

void inputhandle(int argc,char **argv)
{
    char opt;
    while ((opt=getopt(argc,argv,"s:E:b:t:vh"))!=-1)
    {
        switch (opt)
        {
        case 'h':
            print_help();
            helpflag=1;
            break;
        case 'v':
            verbose=1;
            break;
        case 's':
            Cache.s=atol(optarg);
            break;
        case 'E':
            Cache.E=atol(optarg);
            break;
        case 'b':
            Cache.b=atol(optarg);
            break;
        case 't':
            file=fopen(optarg,"r");
            break;
        
        default:
            break;
        }
    }
}

unsigned long SEtoseq(unsigned long s,unsigned long E)
{
    return s*Cache.E+E;
}

unsigned long seqtoS(unsigned long seq)
{
    return seq / Cache.E;
}

unsigned long seqtoE(unsigned long seq)
{
    return seq % Cache.S;
}

int cacheaccess(unsigned long tag,unsigned long S)
{
    int res=-1;
    int emptyspace =  -1;
    int maxspace = -1;
    int maxlru=-1;
    for(int i=0;i<Cache.E;i++)
    {
        if(Cache.cache[SEtoseq(S,i)][VALID])
        {
            //块有效
            if(Cache.cache[SEtoseq(S,i)][TAG]==tag)
            {
                //命中
                Cache.cache[SEtoseq(S,i)][LRUCOUNTER]=0;
                res = HIT;
            }
            else
            {
                //该块未命中
                Cache.cache[SEtoseq(S,i)][LRUCOUNTER]++;
                if((signed)Cache.cache[SEtoseq(S,i)][LRUCOUNTER]>maxlru)
                {
                    maxlru=Cache.cache[SEtoseq(S,i)][LRUCOUNTER];
                    maxspace=i;
                }
            }
        }
        else
        {
            //块无效
            emptyspace = i;
        }
    }
    if(res==HIT) return res;
    else
    {
        if(emptyspace!=-1)
        {
            //有空闲块
            Cache.cache[SEtoseq(S,emptyspace)][VALID]=1;
            Cache.cache[SEtoseq(S,emptyspace)][TAG] =tag;
            Cache.cache[SEtoseq(S,emptyspace)][LRUCOUNTER]=0;
            res=MISS;
        }
        else
        {
            //无空闲块
            Cache.cache[SEtoseq(S,maxspace)][VALID]=1;
            Cache.cache[SEtoseq(S,maxspace)][TAG] =tag;
            Cache.cache[SEtoseq(S,maxspace)][LRUCOUNTER]=0;
            res=EVICTION;
        }
    }
    return res;
}

void reshandle(int res)
{
    switch (res)
    {
    case HIT:
        /* code */
        Hits++;
        if(verbose) printf(" hit");
        break;
    case MISS:
        Misses++;
        if(verbose) printf(" miss");
        break;
    case EVICTION:
        Misses++;
        Evictions++;
        if(verbose) printf(" miss eviction");
        break;
    default:
        break;
    }
}


int main(int argc,char* argv[])
{
    // //printSummary(0, 0, 0);

    char op[3];
    unsigned long tag=0;
    unsigned long S=0;
    int res=0;
    inputhandle(argc,argv);
    if(helpflag) return 0;
    creatCache();
    unsigned long addr,size;
    while (fscanf(file,"%s %lx,%lu",op,&addr,&size)!=EOF)
    {
        /* code */
        tag=addr>>(Cache.s+Cache.b);
        S=(((1<<(Cache.s+Cache.b))-1)&addr)>>Cache.b;
        switch (op[0])
        {
        case 'M':
            /* code */
            if(verbose) printf("%c %lx,%lu",op[0],addr,size);
            res=cacheaccess(tag,S);
            reshandle(res);
            res=cacheaccess(tag,S);
            reshandle(res);
            if(verbose) printf("\n");
            break;
        case 'L':
            if(verbose) printf("%c %lx,%lu",op[0],addr,size);
            res=cacheaccess(tag,S);
            reshandle(res);
            if(verbose) printf("\n");
            break;
        case 'S':
            if(verbose) printf("%c %lx,%lu",op[0],addr,size);
            res=cacheaccess(tag,S);
            reshandle(res);
            if(verbose) printf("\n");
            break;
        case 'I':
            if(verbose) printf("%c %lx,%lu",op[0],addr,size);
            if(verbose) printf("\n");
        }
    }

    printSummary(Hits,Misses,Evictions);
    freeCache();
    return 0;
}
