#include <stdio.h>
#include "lexer.h"
#include <stdlib.h>
#include "parser.h"
#include "sema.h"
int main(int argc, char* argv[])
{
  if (argc < 2) {
    printf("Error: No input file provided.\n");
    return 1;
  }

  char* path = argv[1];
  FILE *f = fopen(path, "rb");
  if (!f) {
    printf("Error: Could not open file '%s'\n", path);
    return 1;
  }

  if (fseek(f, 0, SEEK_END) != 0) { 
    printf("Error: Failed to seek end of file.\n");
    fclose(f); 
    return 1; 
  }

  long size = ftell(f);
  if (size < 0) { 
    printf("Error: Could not determine file size.\n");
    fclose(f); 
    return 1; 
  }
  rewind(f);

  source = malloc((size_t)size + 1);
  if (!source) { 
    printf("Error: Memory allocation failed for file buffer.\n");
    fclose(f); 
    return 1; 
  }

  size_t nread = fread(source, 1, (size_t)size, f);
  if (nread != (size_t)size && !feof(f)) { 
    printf("Error: Failed to read full file content.\n");
    free(source); 
    fclose(f); 
    return 1; 
  }
  source[nread] = '\0';
  fclose(f);

  printf("File loaded successfully (%ld bytes). Initializing lexer...\n", size);
  Token * tokens = init_lexer(source);
  
  printf("\n--- TOKEN STREAM ---\n");
  printf("TYPE\tINDEX\tLENGTH\tLINE\n");
  int i = 0;
  while(tokens[i].type!=T_EOF)
  {
    printf("%d\t%d\t%d\t%d\n", tokens[i].type, tokens[i].ind, tokens[i].len, tokens[i].line);
    i++;
  }
  printf("%d\t%d\t%d\t%d\n", tokens[i].type, tokens[i].ind, tokens[i].len, tokens[i].line);

  printf("starting parsing");
  parse_top_level();
  
  printf("parsing done completly!!\n");

  for (int i = 0; i < gt_count; i++) {
    printf("\n--- AST: %s (ID: %d) ---\n",global_table[i].name, global_table[i].id);
    print_tree(global_table[i].p, 0);
  }
  sema_run_global_analysis();
  
  free(source);
  return 0;
}
