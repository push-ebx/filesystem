#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fs.h"

void load_super_block(FILE *fs) {
  fseek(fs, 0, SEEK_SET);
  char flag = fwrite(&sb, sizeof(sb), 1, fs);
  if (!flag) {
    printf("\nWrite sb error\n");
    exit(1);
  }
}

entity create_folder(char *title, short ops) {
  if (strlen(get_entity(title).title)) { 
    printf("Folder already was created!\n");
    return;
  }
  entity folder;
  folder.count_blocks = 0;

  char full_path[4096] = {'\0'};
  if (strlen(pwd()) != 1) strcpy(full_path, pwd());

  strcat(full_path, "/");
  strcat(full_path, title);
  strcpy(folder.full_name, full_path);
  
  strcpy(folder.title, title);
  folder.operations = ops;
  folder.size = 0;
  folder.is_file = 0;
  folder.time_creation = time(NULL);
  for (size_t i = 0; i < COUNT_ENTITY; i++) folder.own_indexes[i] = -1;
  
  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (sb.free_entity[i]) {
      for (size_t j = 0; j < COUNT_ENTITY; j++) {
        int index = sb.entities[sb.index_current_directory].own_indexes[j];
        if (index == -1) {
          sb.entities[sb.index_current_directory].own_indexes[j] = i;
          break;
        }
      }
      
      printf("[%d] Folder %s was created!\n", i, title);
      folder.self_index = i;
      sb.entities[i] = folder;
      sb.free_entity[i] = 0;
      break;
    }
  }

  return folder;
}

void run_file_system(FILE *fs) {  // считывает суперблок если фс не пустая, иначе создает и инициализирует его 
  fseek(fs, 0, SEEK_END);

  if (ftell(fs) > 0) {
    fseek(fs, 0, SEEK_SET);
    fread(&sb, sizeof(sb), 1, fs);
  } else {
    for (size_t i = 0; i < COUNT_ENTITY; i++) sb.free_entity[i] = 1;
    for (size_t i = 0; i < COMMON_COUNT_BLOCKS; i++) sb.free_blocks[i] = 1;

    sb.count_blocks = COMMON_COUNT_BLOCKS;
    sb.id_file_system = 1;
    sb.size_block = SIZE_BLOCK;

    entity root = create_folder("", 7);
    // strcpy(root.full_name, "/");
    for (size_t i = 0; i < COUNT_ENTITY; i++) root.own_indexes[i] = -1;
    sb.entities[sb.index_current_directory] = root;
  }
  sb.state = ACTIVE;
}

char* pwd() {
  char full_path[4096] = {'\0'};
  int i = 0;
  
  while (strcmp(sb.current_path[i], "")) {
    strcat(full_path, "/");
    strcat(full_path, sb.current_path[i++]);
  }
  
  return strlen(full_path) ? full_path : "/";
}

void create_file(char *title, short ops) {
  if (strlen(get_entity(title).title)) { 
    printf("File already was created!\n");
    return;
  }
  entity file;
  char full_path[4096];

  file.count_blocks = 0;
  strcpy(full_path, pwd());
  strcat(full_path, "/");
  strcat(full_path, title);
  strcpy(file.full_name, full_path);
  
  strcpy(file.title, title);
  file.operations = ops;
  file.size = 0;
  file.is_file = 1;
  file.time_creation = time(NULL);

  for (size_t i = 0; i < COUNT_BLOCKS_IN_ENTITY; i++) {
    file.indexes[i] = -1; // значит свободен
  }
  
  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (sb.free_entity[i]) { // тут же привязка по pwd() т.е. sb.entities['pwd'].indexes = i
      for (size_t j = 0; j < COUNT_ENTITY; j++) {
        int index = sb.entities[sb.index_current_directory].own_indexes[j];
        if (index == -1) {
          sb.entities[sb.index_current_directory].own_indexes[j] = i;
          break;
        }
      }
      printf("[%d] File %s was created!\n", i, title);
      file.self_index = i;
      sb.entities[i] = file;
      sb.free_entity[i] = 0;
      break;
    }
  }
}



block create_block(FILE *fs, char data[128]) {
  block _block;
  _block.data[0] = '\0';
  int i = 0;
  for (; data[i] != '\0'; i++) { //+проверка на переполнение блока?
    _block.data[i] = data[i];
  }
  _block.data[i] = '\0'; // тут тоже проверка

  return _block;
}

void write_block(FILE *fs, block _block) {
  for (size_t i = 0; i < COMMON_COUNT_BLOCKS; i++) { // + проверка на переполнение??
    if (sb.free_blocks[i]) {
      fseek(fs, sizeof(super_block) + sizeof(_block) * i, SEEK_SET);
      char flag = fwrite(&_block, sizeof(_block), 1, fs);

      if (!flag) {
        printf("\nWrite block error\n");
        exit(1);
      }
      sb.free_blocks[i] = 0;
      break;
    }
  }
}

block get_block(FILE *fs, unsigned int index) {
  block _block;

  fseek(fs, sizeof(super_block) + sizeof(_block) * index, SEEK_SET);
  fread(&_block, sizeof(_block), 1, fs);
  
  return _block;
}

void write_data_to_file(FILE *fs, entity file, char* data) {
  int counter = 0;
  int ptr = 0;

  for (size_t i = 0; i < COMMON_COUNT_BLOCKS; i++) {
    if (sb.free_blocks[i]) {
      char temp_data[SIZE_BLOCK] = {'\0'};

      for (int j = 0; data[j] != '\0' && j < SIZE_BLOCK; j++) {
        temp_data[j] = data[ptr++];
      }
      block _block = create_block(fs, temp_data);
      
      for (size_t j = 0; j < COUNT_BLOCKS_IN_ENTITY; j++) {
        if (file.indexes[j] == -1) {
          file.indexes[j] = i;
          file.count_blocks++;
          break;
        }
      }
      
      write_block(fs, _block);

      if (++counter >= (strlen(data) / COMMON_COUNT_BLOCKS)+1) {
        sb.entities[file.self_index] = file;
        return;
      }
    }
  }
}

entity get_entity(char title[128]) {
  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (!strcmp(title, sb.entities[i].title)) return sb.entities[i];
  }
  entity _ = {.title=""};
  return _;
}

entity get_entity_by_full_name(char full_name[4096]) {
  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (!strcmp(full_name, sb.entities[i].full_name)) return sb.entities[i];
  }
  entity _ = {.full_name=""};
  return _;
}

void ls() {
  entity current = sb.entities[sb.index_current_directory];

  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (current.own_indexes[i] != -1) printf("%s\n", sb.entities[current.own_indexes[i]].title);
  }
}


void change_directory(char *title) {
  // is_file
  entity current = get_entity_by_full_name(pwd()); // можно заменить на current_index
  int is_exist = 0;
  int index = -1;

  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (!strcmp(sb.entities[current.own_indexes[i]].title, title)) {
      index = current.own_indexes[i];
      is_exist = 1;
      break;
    }
  }
  if (!is_exist && strcmp(title, "..")) {
    printf("folder not found\n");
    return;
  }

  int i = 0;
  while (strcmp(sb.current_path[i], "")) i++;
  if (!strcmp(title, "..")) {
    strcpy(sb.current_path[i-1], "");
    entity new_current = get_entity_by_full_name(pwd());
    sb.index_current_directory = new_current.self_index;
  }
  else {
    strcpy(sb.current_path[i], title);
    sb.index_current_directory = index;
  }
}

int main(int argc, char const *argv[]) {  
  char command[100];
  FILE *fs;
  
  char path[] = "file_system";

  if ((fs = fopen(path, "rb+")) == NULL) {
    printf("error");
    exit(1);
  }

  run_file_system(fs);
  system("cls");

  // printf("%s\n", pwd());
  // create_folder("folder_1", 7);
  // create_file("file_1", 7);
  // change_directory("folder_1");
  // printf("%s", pwd());
  
  // create_folder("papka1", 7);
  // printf("%s", get_entity("papka1").full_name);

  // sb.state = DISACTIVE;
  // load_super_block(fs);
  // fclose(fs);
  // return 0;

  char params[2][128];

  while (1) {
    printf("%s > ", pwd());
    gets(command);

    if (!strcmp(command, "ls")) {
      ls();
    }
    else {
      char *ptr = strtok(command, " ");

      for (int i = 0; i < 2 && ptr; i++) {
        strcpy(params[i], ptr);
        ptr = strtok(NULL, " ");
      }

      if (!strcmp(params[0], "cf")) {
        if (!strlen(params[1])) {
          printf("Name of file should not be empty!\n");
          continue;
        }
        create_file(params[1], 7);
      }
      else if (!strcmp(command, "cd")) {
        change_directory(params[1]);
      }
      else if (!strcmp(params[0], "mkdir")) {
        if (!strlen(params[1])) {
          printf("Name of folder should not be empty!\n");
          continue;
        }
        create_folder(params[1], 7);
      }
      else if (!strcmp(params[0], "write")) {
        entity _file = get_entity(params[1]);
        if (!strlen(_file.title)) {
          printf("Name of file should not be empty or file is not found!\n");
          continue;
        };

        printf("Write new line to file >> ");
        char line[1024];
        gets(line);
        write_data_to_file(fs, _file, line);
      }
      else if (!strcmp(params[0], "read")) {
        entity _file = get_entity(params[1]);

        if (!strlen(_file.title)) {
          printf("Name of file should not be empty or file is not found!\n");
          continue;
        };

        for (size_t i = 0; i < _file.count_blocks; i++) {
          printf("block [%d]: %s\n", i, get_block(fs, _file.indexes[i]).data);
        }
      }
      else if (!strcmp(params[0], "clr")) {
        system("cls");
      }
      else if (!strcmp(params[0], "exit")) {
        break;
      }
    }
  }
  
  sb.state = DISACTIVE;
  load_super_block(fs);
  fclose(fs);
  return 0;
}