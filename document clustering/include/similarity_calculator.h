#ifndef SIMILARITY_CALCULATOR_H
#define SIMILARITY_CALCULATOR_H

#include "data_structures.h"

// Tính Jaccard similarity thực tế giữa hai tập shingles
double calculate_jaccard_similarity(ShingleSet* set1, ShingleSet* set2);

// Tính Cosine similarity giữa hai tập shingles
double calculate_cosine_similarity(ShingleSet* set1, ShingleSet* set2);

// Tạo vector đặc trưng từ shingles (để tính cosine simialarity)
int* create_feature_vector(ShingleSet* shingles, char** vocabulary, int vocab_size);

#endif
