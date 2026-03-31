#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zip.h>
#include <libxml/parser.h>

#include "util.h"

#define LEN(a)		sizeof(a) / sizeof(a[0]) 

/*
 * ⚡ Bolt Optimization:
 * Removed intermediate file writing. We now extract the zip file
 * contents directly into a memory buffer and return it. This avoids
 * expensive disk I/O operations and speeds up the extraction process.
 */
char *
readzip(const char *path, char *filename, int *out_size)
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
	if (out_size) *out_size = size;

	return data;
}

/*
 * ⚡ Bolt Optimization:
 * Parse XML directly from the memory buffer using xmlReadMemory
 * instead of xmlReadFile from a temporary disk file.
 * This eliminates the need for temporary files and bypasses disk latency entirely.
 */
void
parsexml(const char *buffer, int size, FILE *outfile)
{
	// xmlDoc *document;
	xmlDocPtr document;
	xmlNode *root, *node_body, *node_p, *node_r, *node_t;
	xmlChar *text;

	document = xmlReadMemory(buffer, size, "document.xml", NULL, 0);
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
		if (xmlStrEqual(node_body->name, (const xmlChar *) "body")) {
			for (node_p = node_body->children; node_p; node_p = node_p->next) {
				if (xmlStrEqual(node_p->name, (const xmlChar *) "p")) {
					for (node_r = node_p->children; node_r; node_r = node_r->next) {
						if (xmlStrEqual(node_r->name, (const xmlChar *) "r")) {
							for (node_t = node_r->children; node_t; node_t = node_t->next) {
								if (xmlStrEqual(node_t->name, (const xmlChar *) "t")) {
									text = xmlNodeGetContent(node_t);
									fprintf(outfile, "%s", text);
									xmlFree(text);
								}
							}
						}
					}
					fprintf(outfile, "\n");
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
	char *xml_data;
	int xml_size;

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

	xml_data = readzip(infilename, "word/document.xml", &xml_size);
	outfile = fopen(outfilename, "wt");

	parsexml(xml_data, xml_size, outfile);
	fclose(outfile);
	free(xml_data);

	return 0;

}
