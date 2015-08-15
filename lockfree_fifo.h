#ifndef LOCKFREE_FIFO_H_
#define LOCKFREE_FIFO_H_

/* single reader, single writer */

struct lockfree_fifo;

int lockfree_fifo_init(struct lockfree_fifo **fifo,
                       unsigned elem_size,
                       unsigned capacity);
void lockfree_fifo_exit(struct lockfree_fifo *fifo);

int lockfree_fifo_enqueue(struct lockfree_fifo *fifo, const void *elem);
int lockfree_fifo_dequeue(struct lockfree_fifo *fifo, void *elem);

void lockfree_fifo_block_until_writeable(const struct lockfree_fifo *fifo);
void lockfree_fifo_block_until_readable(const struct lockfree_fifo *fifo);

#endif
