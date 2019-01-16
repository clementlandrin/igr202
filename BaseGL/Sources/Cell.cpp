#include "Cell.h"

bool Cell::isInCell(){
  return (X.x<=v.x)&&(v.x<=X.y)&&(Y.x<=v.y)&&(v.y<=Y.y)&&(Z.x<=v.z)&&(v.z<=Z.y);
}
