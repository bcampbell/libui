#include "db.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

// strdup not defined in strict ansi C?
// TODO: a more elegant solution
extern const char* strdup(const char*);

static void randomiseRecord(Record *r);

Record* dbNewRecord()
{
    Record* r;
    r = (Record*)malloc(sizeof(Record));
    randomiseRecord(r);
    return r;
}

void dbFreeRecord(Record *r)
{
    free((void*)r->Name);
}

DB *dbNewDB()
{
    const int initialCap = 32;
    DB* db;
    db = malloc(sizeof(DB));
    db->count = 0;
    db->cap = initialCap;
    db->records = malloc(sizeof(Record*) * db->cap);
    return db;
}

static void grow(DB *db) {
    db->cap *= 2;
    db->records = realloc(db->records, sizeof(Record*) * db->cap);
}

void dbFreeDB(DB *db)
{
    int i;
    for (i=0; i<db->count; i++) {
        dbFreeRecord(db->records[i]);
    }
    free(db->records);
    free(db);
}

void dbAppend(DB *db, Record *r)
{
    if (db->count == db->cap) {
        grow(db);
    }
    db->records[db->count] = r;
    db->count++;
}

int dbCount(DB *db)
{
    return db->count;
}

const Record *dbGet(DB *db, int idx)
{
    return db->records[idx];
}

void dbDelete(DB *db, int idx)
{
    dbFreeRecord(db->records[idx]);
    for(;idx < db->count - 1; idx++) {
        db->records[idx] = db->records[idx+1];
    }
    db->count--;
}

// guff to generate names and stats in bulk, based on the galaxy-generation code
// in the BBC micro game 'Elite'.
static const char *rndName()
{
	static char buf[4 * 2 + 1];
	const char* pairs = "..lexegezacebisousesarmaindirea.eratenberalavetiedorquanteisrion";
	int i;

	// seed
	uint16_t s[3];
	for( i = 0; i < 3; i++ ) {
		s[i] = rand();
	}

	// some are longer (4 pairs instead of 3)
	int len = (s[0] & 64) ? 4 : 3;

	// pick our pairs
	int idx[4];
	for (i = 0; i < 4; i++) {
		idx[i] = 2 * ((s[2] >> 8) & 0x1f);
		// shuffle
		uint16_t tmp = s[0] + s[1] + s[2];
		s[0] = s[1];
		s[1] = s[2];
		s[2] = tmp;
	}

	// compose
	char *p = buf;
	for (i=0; i<len; i++ )
	{
		char c = pairs[idx[i]];
		if (c != '.') {
			*p++ = c;
		}
		c = pairs[idx[i] + 1];
		if (c != '.') {
			*p++ = c;
		}
	}
	buf[0] = toupper(buf[0]);
	*p++ = '\0';
	return strdup(buf);
}

static void randomiseRecord(Record *r)
{
	r->Name = rndName();
	r->Economy = rand() & 7;
	r->Govt = rand() & 7;
	if (r->Govt <= 1 ) {
		r->Economy = r->Economy | 2;
	}
	r->TechLevel = 1 + (rand() & 3) + (r->Economy ^ 7);
	r->TechLevel += r->Govt / 2;
	if (r->Govt & 1) {
		r->TechLevel++;
	}
	r->Population = 8 * 4 * r->TechLevel + r->Economy;
	r->Population += r->Govt + 1;
	r->Turnover = ((r->Economy ^ 7) + 3) * (r->Govt + 4);
	r->Turnover *= r->Population * 8;
	r->Radius = 256 * ((rand() & 15) + 11);
}

const char* dbEconDesc[8] = {"Rich Industrial","Average Industrial","Poor Industrial","Mainly Industrial", "Mainly Agricultural","Rich Agricultural","Average Agricultural","Poor Agricultural"};

const char* dbGovtDesc[8] = {"Anarchy","Feudal","Multi-gov","Dictatorship","Communist","Confederacy","Democracy","Corporate State"};


