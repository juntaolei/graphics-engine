#include "stack.h"
#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>

struct stack*
new_stack()
{
  /*
  Creates a new stack and puts an identity matrix at the top.

  @param: No paramters

  @return: struct stack*
  */
  struct stack* s;
  struct matrix** m;
  struct matrix* i;
  s = (struct stack*)malloc(sizeof(struct stack));

  m = (struct matrix**)malloc(STACK_SIZE * sizeof(struct matrix*));
  i = new_matrix(4, 4);
  ident(i);

  s->size = STACK_SIZE;
  s->top = 0;
  s->data = m;
  s->data[s->top] = i;

  return s;
}

void
push(struct stack* s)
{
  /*
  Puts a new matrix on top of s. The new matrix should be a copy of the curent
  top matrix.

  @param: struct stack*

  @return: void
  */
  struct matrix* m;
  m = new_matrix(4, 4);

  if (s->top == s->size - 1) {
    s->data = (struct matrix**)realloc(
      s->data, (s->size + STACK_SIZE) * sizeof(struct matrix*));
    s->size = s->size + STACK_SIZE;
  }

  copy_matrix(s->data[s->top], m);

  s->top++;
  s->data[s->top] = m;
}

struct matrix*
peek(struct stack* s)
{
  /*
  Returns a reference to the matrix at the top of the stack.

  @param: struct stack*

  @return: struct matrix*
  */
  return s->data[s->top];
}

void
pop(struct stack* s)
{
  /*
  Remove and free the matrix at the top.

  @param: struct stack*

  @return: void
  */
  free_matrix(s->data[s->top]);
  s->top--;
}

void
free_stack(struct stack* s)
{
  /*
  Deallocate all the memory used in the stack.

  @param: struct stack*

  @return: void
  */
  int i;

  for (i = 0; i <= s->top; i++) {
    free_matrix(s->data[i]);
  }
  free(s->data);
  free(s);
}

void
print_stack(struct stack* s)
{
  /*
  Print the given stack.

  @param: struct stack*

  @return: void
  */
  int i;
  for (i = s->top; i >= 0; i--) {

    print_matrix(s->data[i]);
    printf("\n");
  }
}
