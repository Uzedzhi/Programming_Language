#include <time.h>
#include <stdio.h>

#include "../includes/MyLangVars.hpp"
#include "../includes/MyLangDump.hpp"
#include "../MyLibs/sassert.hpp"

int count_graph_files = 0;

void PrintSiteHeader() {
    if (count_graph_files != 0)
        return;
    FILE * fp = fopen(dump_site_name, "w");
    sassert(fp, ERR_PTR_NULL);
    
    fprintf(fp, "<!DOCTYPE html>\n"
                "<html lang=\"ru\">\n"
                "<head>\n"
                "<style>\n"
                "p, h1, h3, ul{\n"
                "margin: 0;\n"
                "}\n"
                "h2 {\n"
                "color: rgb(30, 0, 255);\n"
                "font-weight: bold;\n"
                "margin-bottom: 0px;\n"
                "}\n"
                "h3 {\n"
                "color: #5c525794\n;"
                "padding-left: 130px;\n"
                "}\n"
                "h1 {\n"
                "color: #e43383ff;\n"
                "font-weight: bold;\n"
                "}\n"
                ".images {\n"
                    "position: relative;\n"
                    "display: inline-block;\n"
                    "height: 200px;\n"
                "}\n"
                "</style>\n"
                "<title>my tree dump</title>\n"
                "</head>\n"
                "<body width=\"device-width\">\n"
            );
    fclose(fp);
}

void PrintSiteToes() {
    FILE * fp = fopen(dump_site_name, "a");\
    sassert(fp, ERR_PTR_NULL);\

    print_divider(fp);
    fprintf(fp, "</body>\n"
                "</html");
    fclose(fp);
}

void print_divider(FILE * fp) {
    fprintf(fp, "<h1 style=\"color: #a30f7eff\">");
    for (size_t i = 0; i < 1000; i++) {
        putc('|', fp);
    }
    fprintf(fp, "</h1>\n");
}

void print_to_html(Node_t *tree, bool needs_division, const char * str) {
    sassert(tree,   ERR_PTR_NULL);

    FILE * fp = fopen(dump_site_name, "a");
    sassert(fp, ERR_PTR_NULL);

    if (needs_division == true)
        print_divider(fp);

    fprintf(fp, "<h2>%s</h2>", str);
    fprintf(fp, "<div class=\"images\">\n"
                "<img src=\"graph/graph%d.png\" class=\"img1\">\n</div>\n",  count_graph_files);
    count_graph_files++;
    fclose(fp);
}

void GraphDumpPrintNode(FILE *fp, Node_t *node) {
    if (node == NULL)
        return;

    fprintf(fp, "{\n"
                "rank=0\n"
                "tree_node_info%p[style=\"rounded\", label=<\n"
                "<TABLE BORDER=\"0\" CELLSPACING=\"0\" CELLBORDER=\"0\" BGCOLOR=\"%s\">\n"
                "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> type = </FONT> <FONT POINT-SIZE=\"18\">%s</FONT> </TD></TR>\n", node, "#ffffff", AllValueTypesTxt[node->type]);
    switch(node->type) {
        case TYPE_NUM:
            fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> val = </FONT> <FONT POINT-SIZE=\"18\">%d</FONT></TD></TR>\n", node->value.num);
            break;
        case TYPE_OP:
            fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> val = </FONT> <FONT POINT-SIZE=\"18\">%s</FONT></TD></TR>\n", AllOper[node->value.oper].Dump);
            break;
        case TYPE_VAR:
            fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> val = </FONT> <FONT POINT-SIZE=\"18\">%s</FONT></TD></TR>\n", node->value.str);
            break;
        default:
            fprintf(fp, "unsopported type: TYPE_STR\n");
            break;
    }
    fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"14\" COLOR=\"#64748B\">address: %p</FONT> </TD></TR>\n", node);
    fprintf(fp, "<TR>\n");
    if (node->left == NULL)
        fprintf(fp, "<TD PORT=\"left\" ><FONT COLOR=\"#2563eb\">L</FONT> <FONT COLOR=\"#94a3b8\">(nil)</FONT></TD>\n");
    else
        fprintf(fp, "<TD PORT=\"left\" ><FONT COLOR=\"#2563eb\">L</FONT> <FONT COLOR=\"#94a3b8\">%p</FONT></TD>\n", node->left);
    if (node->right == NULL)
        fprintf(fp, "<TD PORT=\"right\"><FONT COLOR=\"#94a3b8\">(nil)</FONT> <FONT COLOR=\"#db2777\">R</FONT></TD>\n");
    else
        fprintf(fp, "<TD PORT=\"right\"><FONT COLOR=\"#94a3b8\">%p</FONT> <FONT COLOR=\"#db2777\">R</FONT></TD>\n", node->right);
    fprintf(fp, "</TR>\n"
                "</TABLE>>]\n"
                "}\n");
}

LangErr_t create_tree_graph(Node_t *tree) {
    sassert(tree, ERR_PTR_NULL);
    char command[MAX_STR_SIZE] = {};
    snprintf(command, MAX_STR_SIZE - 1, "dot -Gdpi=80 -Tpng -o graph/graph%d.png", count_graph_files);

    FILE *fp = popen(command, "w");
    RET_ASSERT(fp, ERR_UNDEFINED_CMD, "Проверьте установлен ли dot на вашем компьютере");

    fprintf(fp, "digraph {\n"
                "rankdir=TB\n"
                "ranksep=0.5\n"
                "node[shape=box, style=\"rounded,filled\",\n"
                "color=\"#b8caf2ff\",\n"
                "fontname=\"Inter,Helvetica,Arial\",\n"
                "fontcolor=\"#111827\",\n"
                "penwidth=1.2,\n"
                "margin=\"0.06,0.04\"]\n");
    GraphDumpPrintNode(fp, tree);

    print_nodes_to_dump_file(tree, tree, fp);
    fprintf(fp, "}");
    pclose(fp);
    return OK;
}

void print_nodes_to_dump_file(Node_t * node, Node_t * tree, FILE *fp) {
    sassert(fp, ERR_PTR_NULL);

    static int counter = 0;
    if (node == NULL)
        return;

    GraphDumpPrintNode(fp, node);
    counter++;
    if (node->left != NULL) {
        fprintf(fp, "tree_node_info%p:left->tree_node_info%p[color=\"#2563eb\", label=\"L\", fontcolor=\"#0000ff\" , minlen=2]\n", node, node->left);
        print_nodes_to_dump_file(node->left, tree, fp);
    }

    if (node->right != NULL) {
        fprintf(fp, "tree_node_info%p:right->tree_node_info%p[color=\"#db2777\", label=\"R\", fontcolor=\"#ff0000\" , minlen=2]\n", node, node->right);
        print_nodes_to_dump_file(node->right, tree, fp);
    }

    return;
}