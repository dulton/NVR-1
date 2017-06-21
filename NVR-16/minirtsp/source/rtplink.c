
#include "rtplink.h"
#include "vlog.h"

int DLINK_init(DLink_t **DL)
{
	*DL=(DLink_t *)malloc(sizeof(DLink_t));
	if(*DL == NULL){
		VLOG(VLOG_ERROR,"malloc for dlink failed");
		return RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"malloc dlink @ %p",*DL);
	(*DL)->data.entries=0;
	(*DL)->data.last_node= *DL;
	(*DL)->prev=NULL;
	(*DL)->next=NULL;
	return RET_OK;
}

int DLINK_destroy(DLink_t *DL)
{
	DLNode_t *p=DL;
	DLNode_t *q=NULL;
	int i=0,n=DL->data.entries;
	for(i=0;i<=n;i++){
		q=p->next;
		//printf("i=%d %p %p\n",i,p,q);
		free(p);
		p=q;
	}
	return RET_OK;
}

int DLINK_print(DLink_t *DL)
{
	DLNode_t *p;
	p=DL->next;
	printf("\nDLNode length: %d,\nElement:",DL->data.entries);
	while(p)
	{
		printf("%d,",p->data.frame.timestamp);
		p=p->next;
	}
	return 0;
}

int DLINK_bprint(DLink_t *DL)/* inverse output*/
{
	DLNode_t *p;
	p=DL;
	while(p->next)
	{
		p=p->next;
	}
	printf("\nDLNode length: %d,\nBACK Element:",DL->data.entries);
	while(p->prev)
	{
		printf("%d,",p->data.frame.timestamp);
		p=p->prev;
	}

	return 0;
}


int DLINK_insert(DLink_t *DL,int i,DLElem_t *e)
{
	DLNode_t *pe,*p,*pr;
	int j;
	if(i == DLINK_POS_TOP){
		i = DL->data.entries+1;
	}else if(i == DLINK_POS_BOT){
		i = 1;
	}
	if(i<1 || i>(DL->data.entries+1)){
		VLOG(VLOG_ERROR,"DLINK_insert: invalid arg");
		return RET_FAIL;
	}
	pe=(DLNode_t *)malloc(sizeof(DLNode_t));
	if(pe ==NULL){
		printf("malloc failed\n");
		return RET_FAIL;
	}else{
		//printf("malloc @ %p\n",pe);
	}
	memcpy(&pe->data,e,sizeof(DLElem_t));
	pe->prev=NULL;
	pe->next=NULL;
	p=DL->next;
	pr=DL;
	for(j=1;j<i;j++)
	{
		pr=p;
		p=p->next;
	}
	pe->next=p;
	pe->prev=pr;
	pr->next=pe;
	if(p!=NULL)
	{
		p->prev=pe;		
	}
	DL->data.entries++;
	
	return RET_OK;
}

int DLINK_insert_top(DLink_t *DL,DLElem_t *e)
{
	DLNode_t *pe,*p;
	p = (DLNode_t *)DL->data.last_node;
	
	pe=(DLNode_t *)malloc(sizeof(DLNode_t));
	if(pe ==NULL){
		printf("malloc failed\n");
		return RET_FAIL;
	}
	memcpy(&pe->data,e,sizeof(DLElem_t));
	pe->prev=p;
	pe->next=NULL;

	p->next = pe;
	DL->data.entries++;
	DL->data.last_node = pe;
	
	return RET_OK;
}




int DLINK_delete(DLink_t *DL,int i)
{
	DLNode_t *p;
	int j;
	if(i == DLINK_POS_TOP){
		i = DL->data.entries;
	}else if(i == DLINK_POS_BOT){
		i = 1;
	}
	if(i<1 || i>DL->data.entries){
		VLOG(VLOG_ERROR,"DLINK_delete: invalid arg");
		return RET_FAIL;
	}
	p=DL->next;
	for(j=1;j<i;j++)
	{
		p=p->next;
	}
	p->prev->next=p->next;
	if(p->next!=NULL)
	{
		p->next->prev=p->prev;
	
	}
	if(p == DL->data.last_node){
		DL->data.last_node = p->prev;
	}
	free(p);
	DL->data.entries--;	
	return RET_OK;
}

int DLINK_pull(DLink_t *DL,int i,DLElem_t *e)
{
	DLNode_t *p;
	int j;
	
	if(i == DLINK_POS_TOP){
		i = DL->data.entries;
	}else if(i == DLINK_POS_BOT){
		i = 1;
	}
	if(i<1 || i>DL->data.entries){
		VLOG(VLOG_ERROR,"DLINK_get: invalid arg");
		return RET_FAIL;
	}
	p=DL->next;
	for(j=1;j<i;j++)
	{
		p=p->next;
	}
	if(p){
		memcpy(e,&p->data,sizeof(DLElem_t));
	}
	return RET_OK;
}

/**/
int DLINK_check_lost_seq(DLink_t *DL)
{
	DLNode_t *p;
	unsigned int last_seq=0xffffffff;
	int lost_seq = 0;
	p=DL->next;
	while(p)
	{
		if(last_seq != 0xffffffff){
			if((last_seq +1) != p->data.frame.seq){
				lost_seq++;
			}
		}
		last_seq = p->data.frame.seq;
		p=p->next;
	}
	return lost_seq;
}

// insert and sorted by seq number
int DLINK_push_by_seq(DLink_t * DL,DLElem_t * e,void *pos)
{
	DLNode_t *p=DL->next,*q=NULL,*pr=NULL;
	//int max_seq=0;
	// get insert position
	while(p){
		if(e->frame.seq > p->data.frame.seq ){
			break;
		}
		p = p->next;
	}
	// insert position
	q = p->next;
	pr=(DLNode_t *)pos;
	if(pr == NULL){
		VLOG(VLOG_ERROR,"malloc failed");
		return -1;
	}
	memcpy(&pr->data,e,sizeof(DLElem_t));
	p->next = pr;
	pr->next= q;
	pr->prev = p;
	if(q != NULL)
		q->prev = pr;

	DL->data.entries++;
	DL->data.max_seq = e->frame.seq;
	VLOG(VLOG_DEBUG,"dlink insert %d @%p ",e->frame.seq,pos);
	return 0;
}


int DLINK_pull_by_seq_and_del(DLink_t *DL,DLElem_t *elem)
{
	DLNode_t *p=DL->next,*node=NULL,*node1=NULL,*node2=NULL;
	unsigned int min_seq1 = 0x7fffffff;
	unsigned int min_seq2 = 0x7fffffff;
	int min_entry1 =0;
	int min_entry2 =0;	
	int min_entry =0;
	int i;
	int flag=0;
	for(i=1;i<=DL->data.entries;i++){
		if(p->data.frame.seq == 65535) flag++;
		if(p->data.frame.seq == 0) flag++;
		if(p->data.frame.seq > 64800){
			if(p->data.frame.seq < min_seq2){
				node2 = p;
				min_seq2 = p->data.frame.seq;
				min_entry2 = i;
			}
		}
		if(p->data.frame.seq < min_seq1){
			node1 = p;
			min_seq1 = p->data.frame.seq;
			min_entry1 = i;
		}
		p = p->next;
	}
	if(flag == 2){
		node = node2;
		min_entry = min_entry2;
	}else{
		node = node1;
		min_entry = min_entry1;
	}
	if(node == NULL){
		VLOG(VLOG_DEBUG,"INFO:link is empy now!!!");
		return -1;
	}else{
		VLOG(VLOG_DEBUG2,"DBG: pull success,seq:@%d",min_entry);
	}
	memcpy(elem,&node->data,sizeof(DLElem_t));
	// delete this node
	DLINK_delete(DL,min_entry);
	//
	return min_entry;
}


int DLINK_pull_by_seq(DLink_t *DL,DLElem_t *elem)
{
	DLNode_t *p=DL->next,*node=NULL;
	unsigned int min_seq = 0x7fffffff;
	int min_entry =0;
	int i;
	for(i=1;i<=DL->data.entries;i++){
		if(p->data.frame.seq < min_seq){
			node = p;
			min_seq = p->data.frame.seq;
			min_entry = i;
		}
		p = p->next;
	}
	if(node == NULL){
		VLOG(VLOG_DEBUG,"INFO:link is empy now!!!");
		return -1;
	}else{
		VLOG(VLOG_DEBUG2,"DBG: pull success,seq:%d @%d",min_seq,min_entry);
	}
	memcpy(elem,&node->data,sizeof(DLElem_t));
	//
	return min_entry;
}


int DLINK_pull_and_del(DLink_t *DL,int i,DLElem_t *elem)
{
	DLNode_t *p;
	int j;
	
	if(i == DLINK_POS_TOP){
		i = DL->data.entries;
	}else if(i == DLINK_POS_BOT){
		i = 1;
	}
	if(i<1 || i>DL->data.entries){
		VLOG(VLOG_ERROR,"DLINK_pull: invalid arg:%d",i);
		return RET_FAIL;
	}
	p=DL->next;
	for(j=1;j<i;j++)
	{
		p=p->next;
	}
	if(p){
		memcpy(elem,&p->data,sizeof(DLElem_t));
		// delete it
		DLINK_delete(DL,i);
	}else{
		return RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"DLINK_pull: << %d,%d >>,entries:%d",i,j,DL->data.entries);
	return RET_OK;
}



int dlink_test(int argc,char *argv[])
{
	int cmd = 0;
	int pos = 0;
	const char *usage=
		"*************************************************\r\n"\
		"***********DLINK DEBUG **************************\r\n"\
		"*********** 0 quit ****************************\r\n"\
		"*********** 1 insert ****************************\r\n"\
		"*********** 2 delete ****************************\r\n"\
		"*********** 3 print all *************************\r\n"\
		"*********** 4 get data **************************\r\n";
	DLink_t *dlink;
	DLElem_t elem;
	DLINK_init(&dlink);
	while(1){
		printf(usage);
		printf("input your command: ");
		scanf("%d",&cmd);
		switch(cmd){
			case 0:
				DLINK_destroy(dlink);
				exit(1);
			case 1:
				printf("input pos and data: ");
				scanf("%d %d",&pos,&elem.frame.timestamp);
				DLINK_insert(dlink,pos,&elem);
				DLINK_print(dlink);
				break;
			case 2:
				printf("input pos: ");
				scanf("%d",&pos);
				DLINK_delete(dlink,pos);
				DLINK_print(dlink);
				break;
			case 3:
				DLINK_print(dlink);
				break;
			case 4:
				printf("input pos: ");
				scanf("%d",&pos);
				DLINK_pull(dlink,pos,&elem);
				printf("get data:%d\n",elem.frame.timestamp);
				break;
			default:
				break;
		}
		printf("\r\n");
	}

	return 0;
}

