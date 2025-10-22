#ifndef UTLIS_H
#define UTLIS_H

#include "data_structures.h"

// Tạo từ điển từ tập văn bản (để tính cosine similarity)
char** create_vocabulary(Document* docs, int num_docs, int* vocab_size);

// In kết quả phân cụm
void print_clusters(Cluster* clusters, int num_clusters, Document* docs);

#endif