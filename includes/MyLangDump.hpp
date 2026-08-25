#ifndef TREEDUMP_H
#define TREEDUMP_H

#include "MyLangVars.hpp"
#include <stdio.h>

#ifndef dump_site_name 
#define dump_site_name "dump.html"
#endif

#ifdef DEBUG
#define STARTTXTDUMPS() \
    PrintSiteHeader();

#define FINISHTXTDUMPS()\
    PrintSiteToes();
#else
#define STARTTXTDUMPS()
#define FINISHTXTDUMPS()
#endif

#ifdef DEBUG
#define DUMP_LANGNODE(node, str) {\
    create_tree_graph(node);\
    print_to_html(node, true, str);\
}

#define DUMPNODE(node, need_division, ...) {                                \
    DiffTree_t * tree10 = (DiffTree_t *) calloc(1, sizeof(DiffTree_t));     \
    TreeCtorDiff_internal(tree10);                                          \
                                                                            \
    tree10->root = node;                                                    \
    tree10->num_of_nodes = GetNumOfNodes(node);                             \
    create_tree_graph(tree10);                                              \
    char str[MAX_STR_SIZE] = {};                                            \
    snprintf(str, MAX_STR_SIZE - 1, __VA_ARGS__);                           \
    print_to_html(tree10, need_division, str);                              \
                                                                            \
    free(tree10->variables);                                                \
    free(tree10);                                                           \
}
#else
#define DUMP_LANGNODE(node, str)
#define DUMPNODE(node, need_division, ...)
#endif

LangErr_t create_tree_graph(Node_t *tree);
void print_nodes_to_dump_file(Node_t * node, Node_t * tree, FILE *fp);
void print_divider(FILE * fp);
void PrintSiteToes();
void PrintSiteHeader();
void print_to_html(Node_t *tree, bool needs_division, const char * str);
#endif // TREEDUMP_H