struct CST;
struct CST* CST_create( int num_codepoints );
void CST_destroy( struct CST* CST_p );
uint64_t cumulative_sums_lookup( struct CST* CST_p, unsigned int symbol );
void counter_increment( struct CST* CST_p, unsigned int symbol );
