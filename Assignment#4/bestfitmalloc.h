struct block {
    int full;   //block is full
    int size;   //block size          
    struct block *prev;   
    struct block *next;   
};

void *bestfit_malloc(int size);
void bestfit_free(void *ptr);
