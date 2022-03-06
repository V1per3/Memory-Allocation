#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define PROCESS_NAME_LEN 32	/*进程名长度*/
#define MIN_SLICE 10	/*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024	/*内存大小*/
#define DEFAULT_MEM_START 0	/*起始位置*/
/* 内存分配算法 */
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

int flag=0;	/*设置内存大小标志*/
int mem_size=DEFAULT_MEM_SIZE;/*内存大小*/
int pid=0;/*初始pid*/
int ma_algorithm=MA_FF;/*当前分配算法*/

/*描述每一个空闲块的数据结构*/
typedef struct free_block_struct{
    int size;//空闲块大小
    int start_addr;//空闲块起始地址
    struct free_block_struct *next;//指向下一个空闲块
}free_block_type;
/*指向内存中空闲块链表的首指针*/
free_block_type *free_block;

/*每个进程分配到的内存块的描述*/
typedef struct allocated_block_struct{
    int pid;
    int size;
    int start_addr;//进程分配到的内存块的起始地址
    char process_name[PROCESS_NAME_LEN];//进程名
    struct allocated_block_struct *next;
}allocated_block;
/*进程分配内存块链表的首指针*/
allocated_block *allocated_block_head=NULL;

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
free_block_type* init_free_block()
{
    free_block_type* fb=(free_block_type*)malloc(sizeof(free_block_type));
 
    if(fb==NULL){
		printf("No mem\n");
		return NULL;
	}
    fb->size=mem_size;
    fb->start_addr=DEFAULT_MEM_START;
    fb->next=NULL;
    return fb;
}

/*显示菜单*/
void display_menu()
{
    printf("\n");
    printf("1-Set memory size(default=%d)\n",DEFAULT_MEM_SIZE);
    printf("2-Select memory allocation algorithm\n");
    printf("3-New process\n");
    printf("4-Kill process\n");
    printf("5-Display memory usage\n");
    printf("0-Exit\n");
}

/*设置内存的大小*/
int set_mem_size()
{
    int size;
    if(flag)
    {
        printf("Cannot set memory size again.\n");
        return 0;
    }
 
    printf("Total memory size=");
    scanf("%d",&size);
    char c;
    while ((c = getchar()) != EOF && c != '\n');
    if(size>0)
    {
        mem_size=size;
        free_block->size=mem_size;
        flag=1;
        return 1;
    }
    else
        return 0;
}
 

void swap(int *a,int *b)
{
    int t=*a; *a=*b; *b=t;
}
/*按FF算法重新整理内存空闲块链表*/
void rearrange_FF()
{
    free_block_type *p,*np;
    p=free_block;
    int len=0;
    while(p)
    {
        len++;
        p=p->next;
    }
    for(int i=0;i<len;i++)
    {
        p=free_block;
        for(int j=0;j<len-i-1;j++)
        {
            np=p->next;

                if(p->start_addr>np->start_addr)
                {
                    swap(&p->size,&np->size);
                    swap(&p->start_addr,&np->start_addr);
                }
            p=np;
        }
    }
}
/*按BF算法重新整理内存空闲块链表*/
void rearrange_BF()
{
    free_block_type *p,*np;
 
    p=free_block;
    int len=0;
    while(p)
    {
        len++;
        p=p->next;
    }
    for(int i=0;i<len;i++)
    {
        p=free_block;
        for(int j=0;j<len-i-1;j++)
        {
            np=p->next;
			if(p->size>np->size)
            {
                swap(&p->size,&np->size);
                swap(&p->start_addr,&np->start_addr);
            }
            p=np;
        }
    }
}
/*按WF算法重新整理内存空闲块链表*/
void rearrange_WF()
{
    free_block_type *p,*np;
 
    p=free_block;
    int len=0;
    while(p)
    {
        len++;
        p=p->next;
    }
    for(int i=0;i<len;i++)
    {
        p=free_block;
        for(int j=0;j<len-i-1;j++)
        {
            np=p->next;
            if(p->size<np->size)
            {
                swap(&p->size,&np->size);
                swap(&p->start_addr,&np->start_addr);
            }
            p=np;
        }
    }
}

/*按指定的算法整理内存空闲块链表*/
void rearrange(int algorithm)
{
    switch(algorithm)
    {
        case MA_FF:rearrange_FF();break;
        case MA_BF:rearrange_BF();break;
        case MA_WF:rearrange_WF();break;
    }
}
/* 设置当前的分配算法 */
void set_algorithm()
{
    int algorithm;
    printf("1-First Fit\n");
    printf("2-Best Fit\n");
    printf("3-Worst Fit\n");
    scanf("%d",&algorithm);
    char c;
    while ((c = getchar()) != EOF && c != '\n');
    if(algorithm>=1&&algorithm<=3)
    {
        rearrange(algorithm);
        ma_algorithm=algorithm;
    }
    else
        printf("Choice out of range\n");
}

/*分配内存模块*/
int allocate_mem(allocated_block *ab)
{
   int request=ab->size;   //需要的内存大小
   free_block_type *p,*pre;
   pre=NULL;
   p=free_block;
 
   int first=1; //判断是否为头
   while(p)
   {
        if(p->size>=request)
        {
            ab->start_addr=p->start_addr;
            int rest=p->size-request;   //处理碎片，小于10就归并到该内存中
            if(rest<MIN_SLICE)    
            {
                ab->size+=rest;
            }
            else
            {
                free_block_type *new_block=(free_block_type*)malloc(sizeof(free_block_type));
                new_block->size=rest;
                new_block->start_addr=p->start_addr+request;
                new_block->next=p->next;
                ab->next=NULL;
                if(first)
                    free_block=new_block;
                else
                    pre->next=new_block;
            }
            p->size-=ab->size;
            rearrange(ma_algorithm);
            return 1;
        }
        first=0;
        pre=p;
        p=p->next;
   }
   return -1;
	//根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
	// 1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
	// 2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
	// 3. 在成功分配内存后，应保持空闲分区按照相应算法有序
	// 4. 分配成功则返回1，否则返回-1
}

/*创建新的进程，主要是获取内存的申请数量*/
int new_process()
{
    allocated_block *ab=(allocated_block*)malloc(sizeof(allocated_block));
    int size,ret;
    if(ab==NULL) return -1;
    ab->next=NULL;
    pid++;
    sprintf(ab->process_name,"PROCESS_%02d",pid);
    ab->pid=pid;
    printf("Memory for %s:",ab->process_name);
    scanf("%d",&size);
    char c;while ((c = getchar()) != EOF && c != '\n');
    if(size<=0) return 0;
		ab->size=size;
    ret=allocate_mem(ab);
 
    if(ret==1&&allocated_block_head==NULL)
    {
        allocated_block_head=ab;
        return 1;
    }
    else if(ret==1)
    {
        ab->next=allocated_block_head;
        allocated_block_head=ab;
        return 1;
    }
    else
    {
        printf("Allocated failed\n");
        free(ab);
        return -1;
    }
}

//在进程分配链表中寻找指定进程
allocated_block* find_process(int pid)
{
    allocated_block *p;
    p=allocated_block_head;
    while(p)
    {
        if(p->pid==pid) return p;
        p=p->next;
    }
    return NULL;
}

/*将ab所表示的已分配区归还，并进行可能的合并*/
void free_mem(allocated_block *ab)
{
    free_block_type *fbt=(free_block_type*)malloc(sizeof(free_block_type)),*p,*np;
    fbt->size=ab->size;
    fbt->start_addr=ab->start_addr;
    fbt->next=NULL;
 
    p=free_block;
    while(p->next)     //插入队尾
        p=p->next;
    if(p->start_addr+p->size==fbt->start_addr)  //新释放的结点在队尾后则合并
        {
            p->size+=fbt->size;
            p->next=NULL;
        }
    else
        p->next=fbt;
 
    rearrange(2);
    p=free_block;
    while(p->next)    //检查并合并相邻的空闲分区
    {
        np=p->next;
        if(p->start_addr+p->size==np->start_addr)
            {
                p->size=p->size+np->size;
                p->next=np->next;
            }
        else
            p=np;
    }
    rearrange(ma_algorithm);//将空闲链表重新按照当前算法排序
	// 1. 将新释放的结点插入到空闲分区队列末尾
	// 2. 对空闲链表按照地址有序排列
	// 3. 检查并合并相邻的空闲分区
	// 4. 将空闲链表重新按照当前算法排序
}

/*释放ab数据结构节点*/
void dispose(allocated_block *ab)
{
    if(ab==allocated_block_head)
    {
        allocated_block_head=allocated_block_head->next;
        free(ab);
        return;
    }
    allocated_block *pre,*p;
    pre=allocated_block_head;
    p=allocated_block_head->next;
    while(p!=ab)
    {
        pre=p;
        p=p->next;
    }
    pre->next=p->next;
    free(ab);
}

/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process()
{
    allocated_block *ab;
    int pid;
    printf("Kill Process,pid:");
    scanf("%d",&pid);
    char c;while ((c = getchar()) != EOF && c != '\n');
    ab=find_process(pid);
    if(ab!=NULL)
    {
        free_mem(ab);
        dispose(ab);
    }
    else
    {
        printf("wrong pid,try again\n");
    }
}
/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
void display_mem_usage()
{
    free_block_type *fbt=free_block;
    allocated_block *ab=allocated_block_head;
    if(fbt==NULL) return;
    printf("----------------------------------------------------------\n");
    printf("Free memory:\n");
    printf("%20s %20s\n","start_addr","size");
    while(fbt!=NULL)
    {
        if(fbt->size)
            printf("%20d %20d\n",fbt->start_addr,fbt->size);
        fbt=fbt->next;
    }
 
    printf("\nUsed memory:\n");
    printf("%10s %20s %15s %10s\n","pid","Process_name","start_addr","size");
    while(ab!=NULL)
    {
        printf("%10d %20s %10d %15d\n",ab->pid,ab->process_name,ab->start_addr,ab->size);
        ab=ab->next;
    }
	printf("----------------------------------------------------------\n");
}

//退出程序并释放内存空间
void do_exit()
{
    free(allocated_block_head);
    free(free_block);
    printf("end\n");
}
 
int main()
{
    char choice;
    free_block=init_free_block();//初始化空闲区
    while(1)
    {
        display_menu();//显示菜单
        scanf("%c",&choice);//获取用户输入
        char c;
        while ((c = getchar()) != EOF && c != '\n'); //清空缓存区，防止非法输入
        switch(choice)
        {
            case '1':   set_mem_size();break;
            case '2':   set_algorithm();break;
            case '3':   new_process();break;
            case '4':   kill_process();break;
            case '5':   display_mem_usage();break;
            case '0':   do_exit();exit(0);
            default: break;
        }
    }
    return 0;
}