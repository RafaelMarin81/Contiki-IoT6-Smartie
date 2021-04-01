
struct condition {
  int con_type;
  int con_value;
  char *con_unit;
};

struct access_right{
	char *action;
	char *resource;
	unsigned int flag;
	struct condition conditions [2];
};
struct CAPABILITY_TOKEN_T {
  char *id;
  unsigned int ii;
  char *is;
  char *su;
  char *de;
  char *si;
  unsigned int nb;
  unsigned int na;
  struct access_right rights [1];
}__attribute__ ((__packed__));
typedef struct CAPABILITY_TOKEN_T capability_token_t;
