#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 2

struct stack
{
  int size;
  int top;
  struct matrix** data;
};

struct stack*
new_stack();

struct matrix*
peek(struct stack*);

void
push(struct stack*);

void
pop(struct stack*);

void
free_stack(struct stack*);

void
print_stack(struct stack*);

#endif
