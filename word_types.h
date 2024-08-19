#ifndef WORD_TYPES_H
#define WORD_TYPES_H

typedef enum {
    FIRST_WORD, DIRECT_WORD, IMIDIATE_WORD, REGISTER_WORD, DIRECTIVE_WORD
} wordType;


typedef struct {
    unsigned int are : 3;
    unsigned int destenation_address : 4;
    unsigned int source_address : 4;
    unsigned int opcode : 4;
} firstWord;

typedef struct {
    unsigned int are : 3;
    unsigned int address : 12;
} directWord;

typedef struct {
    unsigned int are : 3;
    unsigned int value : 12;
} imidiateWord;

typedef struct {
    unsigned int are : 3;
    unsigned int source_regiester_name : 3;
    unsigned int destination_register_name : 3;
    unsigned int padding : 6; 
} registerWord;

typedef struct {
    unsigned int value : 15;
} directiveWord;

typedef struct {
    union{
        firstWord first_word;
        directWord direct_word;
        imidiateWord imidiate_word;
        registerWord register_word;
        directiveWord directive_word;
    } type;
    wordType word_type;
} Word;

#endif