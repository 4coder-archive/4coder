/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.03.2016
 *
 * generic dynamically linked list 
 *
 */

// TOP

#if !defined(FRED_LINKED_NODE_MACROS_H)
#define FRED_LINKED_NODE_MACROS_H

#if 0
#define dll_init_sentinel(s) (s)->next=(s),(s)->prev=(s)
#define dll_insert(p,n)      (n)->next=(p)->next,(n)->prev=(p),(p)->next=(n),(n)->next->prev=(n)
#define dll_insert_back(p,n) (n)->prev=(p)->prev,(n)->next=(p),(p)->prev=(n),(n)->prev->next=(n)
#define dll_remove(n)        (n)->next->prev=(n)->prev,(n)->prev->next=(n)->next,(n)->next=(n)->prev=0

#define zdll_push_back_(f,l,n) if(f==0){n->next=n->prev=0;f=l=n;}else{n->prev=l;n->next=0;l->next=n;l=n;}
#define zdll_push_back(f,l,n) Stmnt( zdll_push_back_((f),(l),(n)) )
#define zdll_push_front_(f,l,n) if(f==0){n->prev=n->next=0;f=l=n;}else{n->next=l;n->prev=0;l->prev=n;l=n;}
#define zdll_push_front(f,l,n) Stmnt( zdll_push_front_((f),(l),(n)) )
#define zdll_remove_front_(f,l,n) if(f==l){f=l=0;}else{f=f->next;f->prev=0;}
#define zdll_remove_back_(f,l,n) if(f==l){f=l=0;}else{l=l->prev;l->next=0;}
#define zdll_remove_(f,l,n) if(f==n){zdll_remove_front_(f,l,n);}else if(l==n){zdll_remove_back_(f,l,n);}else{dll_remove(n);}
#define zdll_remove(f,l,n) Stmnt( zdll_remove_((f),(l),(n)) )

#define sll_clear(f,l) (f)=(l)=0
#define sll_push(f,l,n) Stmnt( if((f)==0&&(l)==0){(f)=(l)=(n);}else{(l)->next=(n);(l)=(n);}(l)->next=0; )
#define sll_pop(f,l) Stmnt( if((f)!=(l)){(f)=(f)->next;}else{(f)=(l)=0;} )

#define sll_init_sentinel(s) Stmnt( (s)->next=(s); )
#define sll_insert(p,v) Stmnt( (v)->next=(p)->next; (p)->next = (v); )
#define sll_remove(p,v) Stmnt( Assert((p)->next == (v)); (p)->next = (v)->next; )

struct Node{
    Node *next;
    Node *prev;
};
#endif

#endif

// BOTTOM

