#define DISACTIVE 0
#define ACTIVE 1
#define COMMON_COUNT_BLOCKS 1024
#define SIZE_BLOCK 128
#define COUNT_ENTITY 128
#define COUNT_BLOCKS_IN_ENTITY 8

typedef struct entity {
  char title[128];
  time_t time_creation;
  short is_file;
  char full_name[4096];
  unsigned int count_blocks;
  int indexes[COUNT_BLOCKS_IN_ENTITY];
  unsigned int count_entities;
  unsigned int self_index; // id сущности, индекс в sb
  int own_indexes[COUNT_ENTITY];
} entity;

typedef struct super_block {
  unsigned int count_blocks;
  unsigned int free_blocks[COMMON_COUNT_BLOCKS];
  unsigned int free_entity[COUNT_ENTITY]; // [1] - свободен
  unsigned int id_file_system;
  entity entities[COUNT_ENTITY];
  char current_path[COUNT_ENTITY][128];
  unsigned int index_current_directory;
} super_block;

super_block sb;

typedef struct block {
  char data[SIZE_BLOCK];
  unsigned int end_data;
} block;

entity get_entity(char title[128]);
char* pwd();