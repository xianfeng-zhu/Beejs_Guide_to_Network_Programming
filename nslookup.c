#include "base/base.h"

int main(int argc, char const *argv[]) {
  if (argc != 2)
  {
    fprintf(stderr, "need one and only one hostname");
    return 1;
  }
  return nslookup(argv[1]);
}
