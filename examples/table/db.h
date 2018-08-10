#ifndef __DB_H__
#define __DB_H__

// A contrived database of planets

typedef struct Record Record;
struct Record {
	const char* Name;
	int Economy;    // type of economy 0..7
	int Govt;       // type of government 0..7
	int TechLevel;
	int Turnover;
	int Radius;     // in Km
	int Population; // in 10s of billions
};

// description strings for Economy and Govt fields
const char* dbEconDesc[8];
const char* dbGovtDesc[8];

// dbNewRecord creates a new record (initialised with
// randomly-generated data
Record* dbNewRecord();

// dbFreeRecord destroys Record r
void dbFreeRecord(Record *r);

typedef struct DB DB;
struct DB {
    Record** records;
    int count;
    int cap;
};

// dbNewDB creates a new (empty) database
DB* dbNewDB();

// dbFreeDB frees a database and all the Records it contains
void dbFreeDB(DB *db);

// dbAppend adds a record to the end of db.
// Ownership of record is passed to DB
void dbAppend(DB *db, Record *r);

// dbCount returns the number of records in the db
int dbCount(DB *db);

// dbGet returns the Record at position idx in the db
const Record *dbGet(DB *db, int idx);

// dbDelete removes (and frees) the Record at index idx.
// The remaining entries are moved one slot forward.
void dbDelete(DB *db, int idx);

#endif // __DB_H__

