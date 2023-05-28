#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 256

typedef struct Node {
    char character;
    int frequency;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct MinHeap {
    int size;
    int capacity;
    Node** array;
} MinHeap;

Node* create_node(char character, int frequency) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->character = character;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}
long getOriginalFileSize(const char* input_file) {
    FILE* file = fopen(input_file, "r");
    if (file == NULL) {
        printf("Failed to open input file\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    fclose(file);

    return size;
}
long getCompressedFileSize(const char* compressed_file) {
    FILE* file = fopen(compressed_file, "rb");
    if (file == NULL) {
        printf("Failed to open compressed file\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    fclose(file);

    return size;
}
double calculateCompressionRatio(long original_size, long compressed_size) {
    if (original_size == 0) {
        printf("Original file size is zero\n");
        return 0.0;
    }

    return ((double)(original_size - compressed_size) / original_size) * 100.0;
}


MinHeap* create_min_heap(int capacity) {
    MinHeap* min_heap = (MinHeap*)malloc(sizeof(MinHeap));
    min_heap->size = 0;
    min_heap->capacity = capacity;
    min_heap->array = (Node**)malloc(capacity * sizeof(Node*));
    return min_heap;
}
Node* extract_min(MinHeap* min_heap);
void insert_min_heap(MinHeap* min_heap, Node* node);


void swap_nodes(Node** a, Node** b) {
    Node* temp = *a;
    *a = *b;
    *b = temp;
}
Node* build_huffman_tree_from_stats(int frequency[]) {
    MinHeap* min_heap = create_min_heap(MAX_CHAR);
    int i;

    for (i = 0; i < MAX_CHAR; ++i) {
        if (frequency[i] != 0) {
            Node* node = create_node((char)i, frequency[i]);
            insert_min_heap(min_heap, node);
        }
    }

    while (!is_size_one(min_heap)) {
        Node* left = extract_min(min_heap);
        Node* right = extract_min(min_heap);
        Node* parent = create_node('$', left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        insert_min_heap(min_heap, parent);
    }

    return extract_min(min_heap);
}

void min_heapify(MinHeap* min_heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < min_heap->size && min_heap->array[left]->frequency < min_heap->array[smallest]->frequency)
        smallest = left;

    if (right < min_heap->size && min_heap->array[right]->frequency < min_heap->array[smallest]->frequency)
        smallest = right;

    if (smallest != index) {
        swap_nodes(&min_heap->array[index], &min_heap->array[smallest]);
        min_heapify(min_heap, smallest);
    }
}

int is_size_one(MinHeap* min_heap) {
    return min_heap->size == 1;
}

Node* extract_min(MinHeap* min_heap) {
    Node* node = min_heap->array[0];
    min_heap->array[0] = min_heap->array[min_heap->size - 1];
    --min_heap->size;
    min_heapify(min_heap, 0);
    return node;
}
void insert_min_heap(MinHeap* min_heap, Node* node) {
    ++min_heap->size;
    int i = min_heap->size - 1;

    while (i && node->frequency < min_heap->array[(i - 1) / 2]->frequency) {
        min_heap->array[i] = min_heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    min_heap->array[i] = node;
}

MinHeap* build_min_heap(char* text) {
    int frequency[MAX_CHAR] = { 0 };
    int i;

    for (i = 0; text[i] != '\0'; ++i)
        ++frequency[(int)text[i]];

    MinHeap* min_heap = create_min_heap(MAX_CHAR);

    for (i = 0; i < MAX_CHAR; ++i) {
        if (frequency[i] != 0) {
            Node* node = create_node((char)i, frequency[i]);
            insert_min_heap(min_heap, node);
        }
    }

    return min_heap;
}

Node* build_huffman_tree(char* text) {
    MinHeap* min_heap = build_min_heap(text);
    while (!is_size_one(min_heap)) {
        Node* left = extract_min(min_heap);
        Node* right = extract_min(min_heap);
        Node* parent = create_node('$', left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        insert_min_heap(min_heap, parent);
    }
    return extract_min(min_heap);
}

void generate_codes(Node* root, int* code, int depth, char* codes[]) {
    if (root->left == NULL && root->right == NULL) {
        code[depth] = '\0';
        codes[(int)root->character] = (char*)malloc((depth + 1) * sizeof(char));
        strcpy(codes[(int)root->character], (char*)code);
        return;
    }

    code[depth] = '0';
    generate_codes(root->left, code, depth + 1, codes);

    code[depth] = '1';
    generate_codes(root->right, code, depth + 1, codes);
}


void write_stats_file(char* text, char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return;
    }

    int frequency[MAX_CHAR] = { 0 };
    int i;

    for (i = 0; text[i] != '\0'; ++i)
        ++frequency[(int)text[i]];

    for (i = 0; i < MAX_CHAR; ++i) {
        if (frequency[i] != 0)
            fprintf(file, "%c\t%d\n", (char)i, frequency[i]);
    }

    fclose(file);
}
void compress_file(char* input_filename, char* output_filename, char* stats_filename) {
    FILE* input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error opening input file %s\n", input_filename);
        return;
    }

    FILE* output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error opening output file %s\n", output_filename);
        fclose(input_file);
        return;
    }

    fseek(input_file, 0, SEEK_END);
    long input_size = ftell(input_file);
    rewind(input_file);

    char* input_text = (char*)malloc((input_size + 1) * sizeof(char));
    fread(input_text, sizeof(char), input_size, input_file);
    input_text[input_size] = '\0';

    fclose(input_file);

    Node* root = build_huffman_tree(input_text);
    int code[MAX_CHAR] = { 0 };
    char* codes[MAX_CHAR];
    memset(codes, 0, sizeof(codes));
    generate_codes(root, code, 0, codes);

    write_stats_file(input_text, stats_filename);

    int i;

    for (i = 0; input_text[i] != '\0'; ++i) {
        char* char_code = codes[(int)input_text[i]];
        fprintf(output_file, "%s", char_code);
    }

    fclose(output_file);

    for (i = 0; i < MAX_CHAR; ++i) {
        free(codes[i]);
    }

    free(input_text);
}

void decompress_file(char* input_filename, char* output_filename, char* stats_filename) {
    FILE* input_file = fopen(input_filename, "rb");
    if (input_file == NULL) {
        printf("Error opening input file %s\n", input_filename);
        return;
    }

    FILE* output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error opening output file %s\n", output_filename);
        fclose(input_file);
        return;
    }

    fseek(input_file, 0, SEEK_END);
    long input_size = ftell(input_file);
    rewind(input_file);

    unsigned char* buffer = (unsigned char*)malloc(input_size * sizeof(unsigned char));
    fread(buffer, sizeof(unsigned char), input_size, input_file);

    fclose(input_file);

    FILE* stats_file = fopen(stats_filename, "r");
    if (stats_file == NULL) {
        printf("Error opening stats file %s\n", stats_filename);
        free(buffer);
        return;
    }

    int frequency[MAX_CHAR] = { 0 };
    char line[100];
    while (fgets(line, sizeof(line), stats_file)) {
        char character;
        int freq;
        sscanf(line, "%c\t%d", &character, &freq);
        frequency[(int)character] = freq;
    }

    fclose(stats_file);

    Node* root = build_huffman_tree_from_stats(frequency);

    int bit_position = 0;
    Node* current_node = root;

    for (int i = 0; i < input_size; ++i) {
        unsigned char current_byte = buffer[i];

        for (bit_position = 7; bit_position >= 0; --bit_position) {
            int bit = (current_byte >> bit_position) & 1;

            if (bit == 0) {
                current_node = current_node->left;
            }
            else {
                current_node = current_node->right;
            }

            if (current_node->left == NULL && current_node->right == NULL) {
                fprintf(output_file, "%c", current_node->character);
                current_node = root;
            }
        }
    }

    fclose(output_file);
    free(buffer);
}



int main() {
    compress_file("input.txt", "output.huf", "stats.txt");
    decompress_file("output.huf", "output.txt", "stats.txt");
    const char* input_file = "input.txt";
    const char* compressed_file = "output.huf";

    long original_size = getOriginalFileSize(input_file);
    long compressed_size = getCompressedFileSize(compressed_file);

    if (original_size >= 0 && compressed_size >= 0) {
        double compression_ratio = calculateCompressionRatio(original_size, compressed_size);
        printf("Compression ratio: %.2f%%\n", compression_ratio);
    }

    return 0;
}



//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#define MAX_CHAR 256
//
//typedef struct Node {
//    char character;
//    int frequency;
//    struct Node* left;
//    struct Node* right;
//} Node;
//
//typedef struct MinHeap {
//    int size;
//    int capacity;
//    Node** array;
//} MinHeap;
//
//Node* create_node(char character, int frequency) {
//    Node* node = (Node*)malloc(sizeof(Node));
//    node->character = character;
//    node->frequency = frequency;
//    node->left = NULL;
//    node->right = NULL;
//    return node;
//}
//
//MinHeap* create_min_heap(int capacity) {
//    MinHeap* min_heap = (MinHeap*)malloc(sizeof(MinHeap));
//    min_heap->size = 0;
//    min_heap->capacity = capacity;
//    min_heap->array = (Node**)malloc(capacity * sizeof(Node*));
//    return min_heap;
//}
//
//void swap_nodes(Node** a, Node** b) {
//    Node* temp = *a;
//    *a = *b;
//    *b = temp;
//}
//
//void min_heapify(MinHeap* min_heap, int index) {
//    int smallest = index;
//    int left = 2 * index + 1;
//    int right = 2 * index + 2;
//
//    if (left < min_heap->size && min_heap->array[left]->frequency < min_heap->array[smallest]->frequency)
//        smallest = left;
//
//    if (right < min_heap->size && min_heap->array[right]->frequency < min_heap->array[smallest]->frequency)
//        smallest = right;
//
//    if (smallest != index) {
//        swap_nodes(&min_heap->array[index], &min_heap->array[smallest]);
//        min_heapify(min_heap, smallest);
//    }
//}
//
//Node* extract_min(MinHeap* min_heap) {
//    Node* node = min_heap->array[0];
//    min_heap->array[0] = min_heap->array[min_heap->size - 1];
//    --min_heap->size;
//    min_heapify(min_heap, 0);
//    return node;
//}
//
//void insert_min_heap(MinHeap* min_heap, Node* node) {
//    ++min_heap->size;
//    int i = min_heap->size - 1;
//
//    while (i && node->frequency < min_heap->array[(i - 1) / 2]->frequency) {
//        min_heap->array[i] = min_heap->array[(i - 1) / 2];
//        i = (i - 1) / 2;
//    }
//
//    min_heap->array[i] = node;
//}
//
//MinHeap* build_min_heap(char* text) {
//    int frequency[MAX_CHAR] = { 0 };
//    int i;
//
//    for (i = 0; text[i] != '\0'; ++i)
//        ++frequency[(int)text[i]];
//
//    MinHeap* min_heap = create_min_heap(MAX_CHAR);
//
//    for (i = 0; i < MAX_CHAR; ++i) {
//        if (frequency[i] != 0) {
//            Node* node = create_node((char)i, frequency[i]);
//            insert_min_heap(min_heap, node);
//        }
//    }
//
//    return min_heap;
//}
//Node* build_huffman_tree(char* text, int size) {
//    Node* left, * right, * top;
//    MinHeap* min_heap = build_min_heap(text);
//
//    while (min_heap->size != 1) {
//        left = extract_min(min_heap);
//        right = extract_min(min_heap);
//
//        top = create_node('$', left->frequency + right->frequency);
//        top->left = left;
//        top->right = right;
//
//        insert_min_heap(min_heap, top);
//    }
//
//    Node* root = extract_min(min_heap);
//    return root;
//}
//
//void write_stats_file(char* filename, char* text) {
//    FILE* file = fopen(filename, "w");
//    if (file == NULL) {
//        printf("Failed to create stats.txt file.\n");
//        return;
//    }
//
//    int frequency[MAX_CHAR] = { 0 };
//    int i;
//
//    for (i = 0; text[i] != '\0'; ++i)
//        ++frequency[(int)text[i]];
//
//    for (i = 0; i < MAX_CHAR; ++i) {
//        if (frequency[i] != 0)
//            fprintf(file, "%c\t%d\n", (char)i, frequency[i]);
//    }
//
//    fclose(file);
//}
//
//void encode_huffman_tree(Node* root, char* code, int index, FILE* file) {
//    if (root->left) {
//        code[index] = '0';
//        encode_huffman_tree(root->left, code, index + 1, file);
//    }
//
//    if (root->right) {
//        code[index] = '1';
//        encode_huffman_tree(root->right, code, index + 1, file);
//    }
//
//    if (root->left == NULL && root->right == NULL) {
//        fprintf(file, "%c%s\n", root->character, code);
//    }
//}
//
//void write_huffman_file(char* filename, char* text, Node* root) {
//    FILE* file = fopen(filename, "wb");
//    if (file == NULL) {
//        printf("Failed to create output.huf file.\n");
//        return;
//    }
//
//    char code[MAX_CHAR] = { 0 };
//    encode_huffman_tree(root, code, 0, file);
//
//    for (int i = 0; text[i] != '\0'; ++i) {
//        Node* current = root;
//        while (current->left && current->right) {
//            if (text[i] == '0')
//                current = current->left;
//            else if (text[i] == '1')
//                current = current->right;
//            ++i;
//        }
//        --i;
//        fprintf(file, "%c", current->character);
//    }
//
//    fclose(file);
//}
//
//Node* rebuild_huffman_tree(FILE* file) {
//    Node* root = create_node('$', 0);
//    char character;
//    char code[MAX_CHAR];
//    int index = 0;
//
//    while (fscanf(file, "%c%s\n", &character, code) != EOF) {
//        Node* current = root;
//        for (index = 0; code[index] != '\0'; ++index) {
//            if (code[index] == '0') {
//                if (current->left == NULL)
//                    current->left = create_node('$', 0);
//                current = current->left;
//            }
//            else if (code[index] == '1') {
//                if (current->right == NULL)
//                    current->right = create_node('$', 0);
//                current = current->right;
//            }
//        }
//        current->character = character;
//    }
//
//    return root;
//}
//
//void decode_huffman_file(FILE* input_file, FILE* output_file, Node* root) {
//    Node* current = root;
//
//    int bit;
//    while ((bit = fgetc(input_file)) != EOF) {
//        if (bit == '0')
//            current = current->left;
//        else if (bit == '1')
//            current = current->right;
//
//        if (current->left == NULL && current->right == NULL) {
//            fputc(current->character, output_file);
//            current = root;
//        }
//    }
//}
//
//void destroy_huffman_tree(Node* root) {
//    if (root == NULL)
//        return;
//
//    destroy_huffman_tree(root->left);
//    destroy_huffman_tree(root->right);
//    free(root);
//}
//
//int main() {
//    char input_filename[] = "input.txt";
//    char stats_filename[] = "stats.txt";
//    char huffman_filename[] = "output.huf";
//    char output_filename[] = "output.txt";
//
//    // Read input text file
//    FILE* input_file = fopen(input_filename, "r");
//    if (input_file == NULL) {
//        printf("Failed to open input.txt file.\n");
//        return 1;
//    }
//
//    fseek(input_file, 0, SEEK_END);
//    long input_file_size = ftell(input_file);
//    fseek(input_file, 0, SEEK_SET);
//
//    char* input_text = (char*)malloc((input_file_size + 1) * sizeof(char));
//    fread(input_text, sizeof(char), input_file_size, input_file);
//    input_text[input_file_size] = '\0';
//    fclose(input_file);
//
//    // Step 2: Build frequency map and write stats file
//    int frequency[MAX_CHAR] = { 0 };
//    int i;
//    for (i = 0; input_text[i] != '\0'; ++i)
//        ++frequency[(int)input_text[i]];
//
//    FILE* stats_file = fopen(stats_filename, "w");
//    if (stats_file == NULL) {
//        printf("Failed to create stats.txt file.\n");
//        free(input_text);
//        return 1;
//    }
//
//    for (i = 0; i < MAX_CHAR; ++i) {
//        if (frequency[i] != 0)
//            fprintf(stats_file, "%c\t%d\n", (char)i, frequency[i]);
//    }
//
//    fclose(stats_file);
//
//    // Step 3: Build Huffman Tree
//    Node* huffman_tree = build_huffman_tree(input_text, input_file_size);
//    free(input_text);
//
//    // Step 4: Encode and write Huffman file
//    FILE* huffman_file = fopen(huffman_filename, "wb");
//    if (huffman_file == NULL) {
//        printf("Failed to create output.huf file.\n");
//        destroy_huffman_tree(huffman_tree);
//        return 1;
//    }
//
//    char code[MAX_CHAR] = { 0 };
//    encode_huffman_tree(huffman_tree, code, 0, huffman_file);
//    fclose(huffman_file);
//
//    // Step 5: Read stats file, rebuild Huffman Tree, and decode
//    FILE* stats_file2 = fopen(stats_filename, "r");
//    if (stats_file2 == NULL) {
//        printf("Failed to open stats.txt file.\n");
//        destroy_huffman_tree(huffman_tree);
//        return 1;
//    }
//
//    Node* rebuilt_tree = rebuild_huffman_tree(stats_file2);
//    fclose(stats_file2);
//
//    FILE* huffman_file2 = fopen(huffman_filename, "rb");
//    if (huffman_file2 == NULL) {
//        printf("Failed to open output.huf file.\n");
//        destroy_huffman_tree(huffman_tree);
//        destroy_huffman_tree(rebuilt_tree);
//        return 1;
//    }
//
//    FILE* output_file = fopen(output_filename, "w");
//    if (output_file == NULL) {
//        printf("Failed to create output.txt file.\n");
//        destroy_huffman_tree(huffman_tree);
//        destroy_huffman_tree(rebuilt_tree);
//        fclose(huffman_file2);
//        return 1;
//    }
//
//    decode_huffman_file(huffman_file2, output_file, rebuilt_tree);
//
//    fclose(huffman_file2);
//    fclose(output_file);
//
//    destroy_huffman_tree(huffman_tree);
//    destroy_huffman_tree(rebuilt_tree);
//
//    printf("Compression and decompression completed successfully.\n");
//
//    return 0;
//}


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#define NULL ((void*)0)
//
//#define MAX_CHAR 256
//
//typedef struct Node {
//    char character;
//    int frequency;
//    struct Node* left;
//    struct Node* right;
//} Node;
//
//typedef struct MinHeap {
//    int size;
//    int capacity;
//    Node** array;
//} MinHeap;
//
//Node* create_node(char character, int frequency) {
//    Node* node = (Node*)malloc(sizeof(Node));
//    node->character = character;
//    node->frequency = frequency;
//    node->left = NULL;
//    node->right = NULL;
//    return node;
//}
//
//MinHeap* create_min_heap(int capacity) {
//    MinHeap* min_heap = (MinHeap*)malloc(sizeof(MinHeap));
//    min_heap->size = 0;
//    min_heap->capacity = capacity;
//    min_heap->array = (Node**)malloc(capacity * sizeof(Node*));
//    return min_heap;
//}
//
//void swap_nodes(Node** a, Node** b) {
//    Node* temp = *a;
//    *a = *b;
//    *b = temp;
//}
//
//void min_heapify(MinHeap* min_heap, int index) {
//    int smallest = index;
//    int left = 2 * index + 1;
//    int right = 2 * index + 2;
//
//    if (left < min_heap->size && min_heap->array[left]->frequency < min_heap->array[smallest]->frequency)
//        smallest = left;
//
//    if (right < min_heap->size && min_heap->array[right]->frequency < min_heap->array[smallest]->frequency)
//        smallest = right;
//
//    if (smallest != index) {
//        swap_nodes(&min_heap->array[index], &min_heap->array[smallest]);
//        min_heapify(min_heap, smallest);
//    }
//}
//
//Node* extract_min(MinHeap* min_heap) {
//    Node* node = min_heap->array[0];
//    min_heap->array[0] = min_heap->array[min_heap->size - 1];
//    --min_heap->size;
//    min_heapify(min_heap, 0);
//    return node;
//}
//
//void insert_min_heap(MinHeap* min_heap, Node* node) {
//    ++min_heap->size;
//    int i = min_heap->size - 1;
//
//    while (i && node->frequency < min_heap->array[(i - 1) / 2]->frequency) {
//        min_heap->array[i] = min_heap->array[(i - 1) / 2];
//        i = (i - 1) / 2;
//    }
//
//    min_heap->array[i] = node;
//}
//
//MinHeap* build_min_heap(char* text) {
//    int frequency[MAX_CHAR] = { 0 };
//    int i;
//
//    for (i = 0; text[i] != '\0'; ++i)
//        ++frequency[(int)text[i]];
//
//    MinHeap* min_heap = create_min_heap(MAX_CHAR);
//
//    for (i = 0; i < MAX_CHAR; ++i) {
//        if (frequency[i] != 0) {
//            Node* node = create_node((char)i, frequency[i]);
//            insert_min_heap(min_heap, node);
//        }
//    }
//
//    return min_heap;
//}
//
//Node* build_huffman_tree(char* text, int size) {
//    Node* left, * right, * top;
//    MinHeap* min_heap = build_min_heap(text);
//
//    while (min_heap->size != 1) {
//        left = extract_min(min_heap);
//        right = extract_min(min_heap);
//
//        top = create_node('$', left->frequency + right->frequency);
//        top->left = left;
//        top->right = right;
//
//        insert_min_heap(min_heap, top);
//    }
//
//    Node* root = extract_min(min_heap);
//
//    return root;
//}
//
//void generate_codes(Node* root, int codes[], int top, int code_table[][MAX_CHAR]) {
//    if (root->left) {
//        codes[top] = 0;
//        generate_codes(root->left, codes, top + 1, code_table);
//    }
//
//    if (root->right) {
//        codes[top] = 1;
//        generate_codes(root->right, codes, top + 1, code_table);
//    }
//
//    if (root->left == NULL && root->right == NULL) {
//        int i;
//        for (i = 0; i < top; ++i) {
//            code_table[(int)root->character][i] = codes[i];
//        }
//        code_table[(int)root->character][top] = -1;
//    }
//}
//
//void compress_file(const char* input_file, const char* output_file, int code_table[][MAX_CHAR]) {
//    FILE* input = fopen(input_file, "r");
//    FILE* output = fopen(output_file, "wb");
//
//    int code, bit, byte = 0;
//
//    while ((code = fgetc(input)) != EOF) {
//        int* codes = code_table[code];
//
//        for (bit = 0; codes[bit] != -1; ++bit) {
//            byte = byte << 1;
//            byte = byte | codes[bit];
//
//            if (++bit % 8 == 0) {
//                fputc(byte, output);
//                byte = 0;
//            }
//        }
//    }
//
//    if (bit > 0) {
//        byte = byte << (8 - bit);
//        fputc(byte, output);
//    }
//
//    fclose(input);
//    fclose(output);
//}
//
//void decompress_file(const char* input_file, const char* output_file, Node* root) {
//    FILE* input = fopen(input_file, "rb");
//    FILE* output = fopen(output_file, "w");
//
//    Node* current = root;
//    int bit, byte;
//
//    while ((byte = fgetc(input)) != EOF) {
//        for (bit = 7; bit >= 0; --bit) {
//            int value = (byte >> bit) & 1;
//
//            if (value == 0)
//                current = current->left;
//            else
//                current = current->right;
//
//            if (current->left == NULL && current->right == NULL) {
//                fputc(current->character, output);
//                current = root;
//            }
//        }
//    }
//
//    fclose(input);
//    fclose(output);
//}
//
//void save_statistics(const char* output_file, int code_table[][MAX_CHAR]) {
//    FILE* output = fopen(output_file, "w");
//
//    int i, j;
//
//    for (i = 0; i < MAX_CHAR; ++i) {
//        if (code_table[i][0] != -1) {
//            fprintf(output, "%c\t%d\n", (char)i, code_table[i][0]);
//        }
//    }
//
//    fclose(output);
//}
//
//int main() {
//    // Step 1: Read input file
//    FILE* file = fopen("input.txt", "r");
//    if (file == NULL) {
//        printf("Error opening input file.\n");
//        return 1;
//    }
//    fseek(file, 0, SEEK_END);
//    long file_size = ftell(file);
//    fseek(file, 0, SEEK_SET);
//
//    char* text = (char*)malloc(file_size + 1);
//    fread(text, sizeof(char), file_size, file);
//    text[file_size] = '\0';
//
//    fclose(file);
//
//    // Step 2: Generate statistics and save to stats.txt
//    int code_table[MAX_CHAR][MAX_CHAR];
//    memset(code_table, -1, sizeof(code_table));
//
//    Node* root = build_huffman_tree(text, file_size);
//    int codes[MAX_CHAR];
//    generate_codes(root, codes, 0, code_table);
//
//    save_statistics("stats.txt", code_table);
//
//    // Step 3: Compress input.txt and save to output.huf
//    compress_file("input.txt", "output.huf", code_table);
//
//    // Step 4: Decompress output.huf and save to output.txt
//    decompress_file("output.huf", "output.txt", root);
//
//    printf("Compression and decompression completed successfully.\n");
//
//    // Clean up memory
//    free(text);
//    free(root);
//
//    return 0;
//}
//
//


