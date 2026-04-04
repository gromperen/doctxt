#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zip.h>
#include <libxml/parser.h>

#include "util.h"

#define LEN(a)		sizeof(a) / sizeof(a[0]) 

void
readzip(const char *path, const char *filename, char **out_data, int *out_size)
{
	zip_t *zf;
	zip_file_t *file;
	struct zip_stat st;
	int err = 0;
	int size;
	char *data;

	if ((zf = zip_open(path, 0, &err)) == NULL) {
		die("Unable to extract zip: %s", path);
	}

	file = zip_fopen(zf, filename, ZIP_FL_UNCHANGED);

	if (file == NULL) {
		die("File is wrong format");
	}

	zip_stat(zf, filename, 0, &st);
	size = st.size;

	data = ecalloc(sizeof(char), size + 10);

	zip_fread(file, data, size);

	zip_fclose(file);
	zip_close(zf);
	
	data[size] = '\0'; /* fixes bug where sometimes file doesnt end in '\0' */

	*out_data = data;
	*out_size = size;
}

void
parsexml(const char *buffer, int size, FILE *outfile)
{
	xmlDocPtr document;
	xmlNode *root, *node_body, *node_p, *node_r, *node_t;
	xmlChar *text;

	document = xmlReadMemory(buffer, size, "noname.xml", NULL, 0);
	if (document == NULL) {
		die("Unable to read xml file");
	}

	root = xmlDocGetRootElement(document);

	if (root == NULL) {
		die("No root in xml file");
	}

	if (!xmlStrEqual(root->name, (const xmlChar *) "document")) {
		die("wrong xml format");
	}
	for (node_body = root->children; node_body; node_body = node_body->next) {
		if (node_body->type == XML_ELEMENT_NODE && xmlStrEqual(node_body->name, (const xmlChar *) "body")) {
			for (node_p = node_body->children; node_p; node_p = node_p->next) {
				if (node_p->type == XML_ELEMENT_NODE && xmlStrEqual(node_p->name, (const xmlChar *) "p")) {
					for (node_r = node_p->children; node_r; node_r = node_r->next) {
						if (node_r->type == XML_ELEMENT_NODE && xmlStrEqual(node_r->name, (const xmlChar *) "r")) {
							for (node_t = node_r->children; node_t; node_t = node_t->next) {
								if (node_t->type == XML_ELEMENT_NODE && xmlStrEqual(node_t->name, (const xmlChar *) "t")) {
									text = xmlNodeGetContent(node_t);
									fputs((const char*)text, outfile);
									xmlFree(text);
								}
							}
						}
					}
					fputc('\n', outfile);
				}
			}
		}
		break;
	}

	xmlFreeDoc(document);
	xmlCleanupParser();
	return;
}

void
usage()
{
	die("usage: doctxt infile [-o outfile]");
	return;
}

int
main(int argc, char *argv[])
{
	FILE *outfile = NULL;
	char *outfilename = "out.txt";
	char *infilename = "";

	if (argc < 2) {
		usage();
	}
	infilename = argv[1];
	for (int i = 2; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			puts("doctxt-"VERSION);
			return 0;
		} else if (!strcmp(argv[i], "-o")) {
			if (argc <= i - 1) {
				usage();
			}
			outfilename = argv[i + 1];
			i++;
		}
		else {
			usage();
		}
	}

	char *xml_data = NULL;
	int xml_size = 0;
	readzip(infilename, "word/document.xml", &xml_data, &xml_size);
	outfile = fopen(outfilename, "wt");

	parsexml(xml_data, xml_size, outfile);
	fclose(outfile);
	free(xml_data);

	return 0;

}
