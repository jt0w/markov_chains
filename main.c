#include <stdio.h>

#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <time.h>

struct MarkovNode;

typedef struct {
  struct MarkovNode *dest;
  double prob;
  double abs_prob;
} Link;

typedef struct {
  Link *items;
  size_t count;
  size_t cap;

  double total_abs;
} Links;

typedef struct MarkovNode {
  char *name;
  Links links;
} MarkovNode;

typedef struct {
  MarkovNode *items;
  size_t count;
  size_t cap;

  size_t i;
} MarkovChain;

double randomd() {
  double div = RAND_MAX / 1;
  return rand() / div;
}

char *markov_step(MarkovChain *chain) {
  MarkovNode node = chain->items[chain->i];
  MarkovNode *next;
  if (da_len(node.links) == 0) {
    return NULL;
  }
  if (da_len(node.links) == 1)
    assert(node.links.items[0].prob == 1.0);

  double r = randomd();
  double prob = 0.0;
  for (size_t i = 0; i < da_len(node.links); ++i) {
    prob += node.links.items[i].prob;
    if (r <= prob) {
      next = node.links.items[i].dest;
      break;
    }
  }
  assert(next);
  for (size_t i = 0; i < chain->count; ++i) {
    if (strcmp(chain->items[i].name, next->name) == 0) {
      chain->i = i;
      return next->name;
    }
    if (i == chain->count - 1) {
      fprintln(stderr, "FATAL ERROR: Destination of link in Markov Chain is "
                       "not in the chain");
      abort();
    }
  }
  abort();
}

void print_node(MarkovNode node) {
  println("%s:", node.name);
  println("  links:");
  for (size_t i = 0; i < da_len(node.links); ++i) {
    println("    %s: %lf:", node.links.items[i].dest->name,
            node.links.items[i].prob);
  }
}

void print_chain(MarkovChain chain) {
  for (size_t i = 0; i < da_len(chain); ++i) {
    print_node(chain.items[i]);
  }
}

MarkovNode *find_node_by_name(MarkovChain *chain, char *name) {
  for (size_t i = 0; i < chain->count; ++i) {
    if (strcmp(chain->items[i].name, name) == 0) {
     return &chain->items[i];
    }
  }
  return NULL;
}

int find_chain_index_by_name(MarkovChain *chain, char *name) {
  for (size_t i = 0; i < chain->count; ++i) {
    if (strcmp(chain->items[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

int find_link_index(Links *links, MarkovNode *node) {
  for (size_t i = 0; i < links->count; ++i) {
    if (links->items[i].dest == node)
      return i;
  }
  return -1;
}

// Populate chain with all the needed nodes
void populate_chain(MarkovChain *chain, char *input) {
  assert(chain->count == 0);
  for (size_t i = 0; i < strlen(input); ++i) {
    char c = input[i];
    MarkovNode *n = find_node_by_name(chain, &c);
    if (!n) {
      MarkovNode node;
      node.name = temp_sprintf("%c", c);
      da_push(chain, node);
      n = &chain->items[chain->count - 1];
    }
    if (!(chain->count > 0 && i > 0))
      continue;
    // backtracking
    int index = find_chain_index_by_name(chain, temp_sprintf("%c", input[i-1]));
    int lindex = find_link_index(&chain->items[index].links, n);

    if (lindex == -1) {
      da_push(&chain->items[index].links, ((Link){.dest = n, .prob = 1}));
      lindex = chain->items[index].links.count - 1;
    }
    chain->items[index].links.total_abs++;
    chain->items[index].links.items[lindex].abs_prob++;
  }
  for (size_t i = 0; i < chain->count; ++i) {
    for (size_t j = 0; j < chain->items[i].links.count; ++j) {
      // abs_prob / total
      chain->items[i].links.items[j].prob = (chain->items[i].links.items[j].abs_prob) / (chain->items[i].links.total_abs);
    }
  }
}

int main(int argc, char **argv) {
  char *program = shift(argv, argc);
  if (argc != 1) {
    println("USAGE: %s <input.txt>", program);
    fprintln(stderr, "ERROR: input file not provided");
    return 1;
  }

  char *filename = shift(argv, argc);
  StringBuilder sb = {0};
  read_file(filename, &sb);
  da_push(&sb, '\0');

  srand(time(NULL));
  MarkovChain chain = {0};
  populate_chain(&chain, sb.items);

  // MarkovNode A = {.name = "A"};
  // MarkovNode B = {.name = "B"};
  // da_push(&A.links, ((Link){.dest = &A, .prob = 0.5}));
  // da_push(&A.links, ((Link){.dest = &B, .prob = 0.5}));
  //
  // da_push(&B.links, ((Link){.dest = &B, .prob = 0.5}));
  // da_push(&B.links, ((Link){.dest = &A, .prob = 0.5}));
  //
  // da_push(&chain, A);
  // da_push(&chain, B);

  // print_chain(chain);

  printf("%s", chain.items[0].name);
  size_t cap = 10000;
  for (size_t i = 0;i < cap;++i) {
    char *s = markov_step(&chain);
    if (s != NULL)
      printf("%s", s);
    else
      break;
  }
  printf("\n");
  return 0;
}
