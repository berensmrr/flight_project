#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>

// STRUCT 
struct Passenger {
    char ticket_id[8];
    char timestamp[20];
    float baggage_weight;
    int loyalty_points;
    char status[16];
    char destination[31];
    char cabin_class[9];
    int seat_num;
    char app_ver[16];
    char passenger_name[101];
};

// FIRST CHAR HEX 
// UTF 8 first char how much byte calculation
void getFirstCharHex(const char *str, char *hexOut) {
    unsigned char *s = (unsigned char *)str;
    if (s[0] == 0) {
        sprintf(hexOut, "00");
    } else if (s[0] < 0x80) {
        sprintf(hexOut, "%02X", s[0]);
    } else if (s[0] < 0xE0) {
        sprintf(hexOut, "%02X%02X", s[0], s[1]);
    } else if (s[0] < 0xF0) {
        sprintf(hexOut, "%02X%02X%02X", s[0], s[1], s[2]);
    } else {
        sprintf(hexOut, "%02X%02X%02X%02X", s[0], s[1], s[2], s[3]);
    }
}

// CSV TO BINARY 
// r = reading csv wb= writing to binary
void csvToBinary(const char *inputFile, const char *outputFile, char separator) {
    FILE *csvFile = fopen(inputFile, "r");
    if (csvFile == NULL) { 
        printf("CSV file can't open!\n"); 
        return; 
    }

    FILE *binaryFile = fopen(outputFile, "wb");
    if (binaryFile == NULL) { 
        printf("Binary file can't open!\n"); 
        return; 
    }

    char satir[512];
    
    // Header line jump
    // memset = reseting memory and pointers
    // atof= converting to float from string  atoi = converting to integer form string
    fgets(satir, sizeof(satir), csvFile);

    while (fgets(satir, sizeof(satir), csvFile) != NULL) {
        satir[strcspn(satir, "\n\r")] = 0;
        if (strlen(satir) == 0) continue;

        struct Passenger p;
        memset(&p, 0, sizeof(p));

        char *fields[10];
        char *ptr = satir;
        int fieldCount = 0;
        
        for (int i = 0; i < 10; i++) {
            fields[i] = ptr;
            char *next = strchr(ptr, separator);
            if (next != NULL) {
                *next = '\0';
                ptr = next + 1;
            }
            fieldCount++;
            if (next == NULL) break;
        }

        if (fieldCount > 0 && strlen(fields[0]) > 0) strcpy(p.ticket_id, fields[0]);
        if (fieldCount > 1 && strlen(fields[1]) > 0) strcpy(p.timestamp, fields[1]);
        if (fieldCount > 2 && strlen(fields[2]) > 0) p.baggage_weight = atof(fields[2]);
        if (fieldCount > 3 && strlen(fields[3]) > 0) p.loyalty_points = atoi(fields[3]);
        if (fieldCount > 4 && strlen(fields[4]) > 0) strcpy(p.status, fields[4]);
        if (fieldCount > 5 && strlen(fields[5]) > 0) strcpy(p.destination, fields[5]);
        if (fieldCount > 6 && strlen(fields[6]) > 0) strcpy(p.cabin_class, fields[6]);
        if (fieldCount > 7 && strlen(fields[7]) > 0) p.seat_num = atoi(fields[7]);
        if (fieldCount > 8 && strlen(fields[8]) > 0) strcpy(p.app_ver, fields[8]);
        if (fieldCount > 9 && strlen(fields[9]) > 0) strcpy(p.passenger_name, fields[9]);

        fwrite(&p, sizeof(struct Passenger), 1, binaryFile);
    }

    fclose(csvFile);
    fclose(binaryFile);
    printf("CSV → Binary conversion completed!\n");
}

// BINARY TO XML 
// rb = read binary
void binaryToXML(const char *inputFile, const char *outputFile) {
    FILE *binaryFile = fopen(inputFile, "rb");
    if (binaryFile == NULL) { 
        printf("Binary file can't open!\n"); 
        return; 
    }

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "flightlogs");
    xmlDocSetRootElement(doc, root);

    struct Passenger p;
    int id = 1;
    char buffer[64];

    while (fread(&p, sizeof(struct Passenger), 1, binaryFile) == 1) {
        xmlNodePtr entry = xmlNewChild(root, NULL, BAD_CAST "entry", NULL);
        sprintf(buffer, "%d", id++);
        xmlNewProp(entry, BAD_CAST "id", BAD_CAST buffer);

        xmlNodePtr ticket = xmlNewChild(entry, NULL, BAD_CAST "ticket", NULL);
        xmlNewChild(ticket, NULL, BAD_CAST "ticket_id", BAD_CAST p.ticket_id);
        xmlNewChild(ticket, NULL, BAD_CAST "destination", BAD_CAST p.destination);
        xmlNewChild(ticket, NULL, BAD_CAST "app_ver", BAD_CAST p.app_ver);

        xmlNodePtr metrics = xmlNewChild(entry, NULL, BAD_CAST "metrics", NULL);
        xmlNewProp(metrics, BAD_CAST "status", BAD_CAST p.status);
        xmlNewProp(metrics, BAD_CAST "cabin_class", BAD_CAST p.cabin_class);

        sprintf(buffer, "%.1f", p.baggage_weight);
        xmlNewChild(metrics, NULL, BAD_CAST "baggage_weight", BAD_CAST buffer);
        sprintf(buffer, "%d", p.loyalty_points);
        xmlNewChild(metrics, NULL, BAD_CAST "loyalty_points", BAD_CAST buffer);
        sprintf(buffer, "%d", p.seat_num);
        xmlNewChild(metrics, NULL, BAD_CAST "seat_num", BAD_CAST buffer);

        xmlNewChild(entry, NULL, BAD_CAST "timestamp", BAD_CAST p.timestamp);

        // first_char_hex calculation
        char hexBuf[16];
        getFirstCharHex(p.passenger_name, hexBuf);

        // passenger_name node created
        xmlNodePtr pname = xmlNewChild(entry, NULL, BAD_CAST "passenger_name", BAD_CAST p.passenger_name);
        xmlNewProp(pname, BAD_CAST "current_encoding", BAD_CAST "UTF-8");
        xmlNewProp(pname, BAD_CAST "first_char_hex", BAD_CAST hexBuf);
    }

    fclose(binaryFile);
    xmlSaveFormatFileEnc(outputFile, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    printf("Binary → XML conversion completed!\n");
}

// XSD VALIDATION 
void validateXML(const char *xmlFile, const char *xsdFile) {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr c_txt;

    xmlLineNumbersDefault(1);
    c_txt = xmlSchemaNewParserCtxt(xsdFile);
    schema = xmlSchemaParse(c_txt);
    xmlSchemaFreeParserCtxt(c_txt);

    doc = xmlReadFile(xmlFile, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "XML dosyası okunamadı!\n");
        return;
    }

    xmlSchemaValidCtxtPtr vc_txt = xmlSchemaNewValidCtxt(schema);
    int result = xmlSchemaValidateDoc(vc_txt, doc);

    if (result == 0) printf("%s validates\n", xmlFile);
    else if (result > 0) printf("%s fails to validate\n", xmlFile);
    else printf("Internal error\n");

    xmlSchemaFreeValidCtxt(vc_txt);
    xmlFreeDoc(doc);
    if (schema != NULL) xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
}

// ENCODING CONVERSION 
void convertEncoding(const char *inputFile, const char *outputFile, int encoding) {
    xmlDocPtr doc = xmlReadFile(inputFile, NULL, 0);
    if (doc == NULL) { printf("File cant reading!\n"); return; }

    const char *encName;
    if (encoding == 1) encName = "UTF-16LE";
    else if (encoding == 2) encName = "UTF-16BE";
    else if (encoding == 3) encName = "UTF-8";
    else { printf("Invalide encoding!\n"); return; }

    // passenger_name attributes upgrade
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlNodePtr entry = root->children;

    while (entry != NULL) {
        if (entry->type == XML_ELEMENT_NODE) {
            xmlNodePtr child = entry->children;
            while (child != NULL) {
                if (child->type == XML_ELEMENT_NODE &&
                    xmlStrcmp(child->name, BAD_CAST "passenger_name") == 0) {

                    // current_encoding upgrade
                    xmlSetProp(child, BAD_CAST "current_encoding", BAD_CAST encName);

                    // first_char_hex upgrade
                    xmlChar *content = xmlNodeGetContent(child);
                    if (content != NULL && xmlStrlen(content) > 0) {
                        char hexBuf[16];
                        unsigned char *s = (unsigned char *)content;

                        if (encoding == 1) {
                            // UTF-16LE: reverse byte
                            if (s[0] < 0x80) {
                                sprintf(hexBuf, "%02X00", s[0]);
                            } else if (s[0] < 0xE0) {
                                // 2 byte UTF-8 → UTF-16LE
                                int codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
                                sprintf(hexBuf, "%02X%02X", codepoint & 0xFF, (codepoint >> 8) & 0xFF);
                            } else {
                                sprintf(hexBuf, "0000");
                            }
                        } else if (encoding == 2) {
                            // UTF-16BE: normal byte
                            if (s[0] < 0x80) {
                                sprintf(hexBuf, "00%02X", s[0]);
                            } else if (s[0] < 0xE0) {
                                int codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
                                sprintf(hexBuf, "%02X%02X", (codepoint >> 8) & 0xFF, codepoint & 0xFF);
                            } else {
                                sprintf(hexBuf, "0000");
                            }
                        } else {
                            // UTF-8: orijinal hex
                            getFirstCharHex((const char *)content, hexBuf);
                        }

                        xmlSetProp(child, BAD_CAST "first_char_hex", BAD_CAST hexBuf);
                        xmlFree(content);
                    }
                }
                child = child->next;
            }
        }
        entry = entry->next;
    }

    xmlSaveFormatFileEnc(outputFile, doc, encName, 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    printf("%s conversion completed!\n", encName);
}

// HELP 
void printHelp() {
    printf("Usage: ./flightTool <input> <output> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-encoding <1|2|3>]\n");
    printf("conversion_type: 1=CSV->Binary, 2=Binary->XML, 3=XSD Validation, 4=Encoding\n");
    printf("-separator: 1=comma, 2=tab, 3=semicolon\n");
    printf("-opsys: 1=windows, 2=linux, 3=macos\n");
    printf("-encoding: 1=UTF-16LE, 2=UTF-16BE, 3=UTF-8\n");
}

// MAIN
int main(int argc, char *argv[]) {
    if (argc < 2) { printHelp(); return 1; }

    if (strcmp(argv[1], "-h") == 0) { printHelp(); return 0; }

    if (argc < 4) { printf("Incompleted argument!\n"); printHelp(); return 1; }

    char *inputFile  = argv[1];
    char *outputFile = argv[2];
    int convType     = atoi(argv[3]);

    char separator = ',';
    int opsys = 2;
    int encoding = 0;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-separator") == 0 && i+1 < argc) {
            int sep = atoi(argv[++i]);
            if (sep == 1) separator = ',';
            else if (sep == 2) separator = '\t';
            else if (sep == 3) separator = ';';
        } else if (strcmp(argv[i], "-opsys") == 0 && i+1 < argc) {
            opsys = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-encoding") == 0 && i+1 < argc) {
            encoding = atoi(argv[++i]);
        }
    }

    if (convType == 1) {
       csvToBinary(inputFile, outputFile, separator);
    } else if (convType == 2) {
       binaryToXML(inputFile, outputFile);
    } else if (convType == 3) {
       validateXML(inputFile, outputFile);
    } else if (convType == 4) {
       convertEncoding(inputFile, outputFile, encoding);
    } else {
       printf("Unvalide conversion type!\n");
       printHelp();
    }

    return 0;
}