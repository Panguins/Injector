#pragma once
#include <cstdlib>
#include <ctime>

class c_encryption {
public:
	static void generate_key( unsigned char* key, size_t length ) {
		//this will be on server, without the lame way of initializing seed
		static bool rand_seed_set = false;
		if ( !rand_seed_set ) {
			srand( (unsigned int)time( nullptr ) );
			rand_seed_set = true;
		}

		for ( size_t i { }; i < length; i++ )
			*key = (unsigned char)( rand( ) % 256 );
	}

	static void xor( unsigned char* data, size_t data_size, unsigned char* key, size_t key_length, size_t start = 0 ) {
		for ( size_t i { }; i < data_size; i++ ) {
			size_t encrypt_i = i + start;
			char current_key = key[encrypt_i % key_length];
			data[i] ^= current_key;
		}
	}
};
