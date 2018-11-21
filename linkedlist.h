#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

typedef struct node Node;

typedef struct list List;

List * makelist();
void add(char * data, List * list);
void delete(char * data, List * list);
void display(List * list);
void reverse(List * list);
void destroy(List * list);

#endif
