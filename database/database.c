// database.c

// Database Reader
// Dustin Holliday
// 02/04/2022

// This is a simple program that reads from a database called "user_data"
// in a table called "log_1" and prints the table back out to stdout

// Takes command line arguments -s <START DATE> and -e <END DATE>
// Dates are in format "YYYY-MM-DD"
// and displays entries in the table within these dates (inclusive)

// IP Address: 192.168.145.68

// User ID: process_user
// Password: secret_password

#include "/usr/include/mysql/mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IP "192.168.145.68"
#define PORT 3306
#define USER "process_user"
#define PASSWORD "secret_password"
#define DB "user_data"


void sqlInit(MYSQL *);
void sqlConnect(MYSQL *);
MYSQL_RES *sqlQuery(MYSQL *, char *);
int parseArgs(char*, char*, int, char**);
void processQuery(MYSQL_RES *, char [][100]);

int main(int argc, char *argv[])
{
	// Storing our field names for display
	// Storing them makes it easier for processing later
	char fields[4][100];
	strcpy(fields[0], "date");
	strcpy(fields[1], "time");
	strcpy(fields[2], "message");
	strcpy(fields[3], "pgm_name");

	MYSQL mysql;
	MYSQL_RES *query_result;

	char start_date[100], end_date[100];

	// Ensure proper number of arguments used
	if (argc != 5)
	{
		printf("usage: ./database -s <START DATE> -e <END DATE>\n");
		exit(1);
	}

	// Parse arguments into our start_date and end_date variables
	parseArgs(start_date, end_date, argc, argv);

	sqlInit(&mysql);
	sqlConnect(&mysql);

	// Build our query
	// This probably is vulnerable to a SQL injection but I'm assuming
	// this will be on an air-gapped local network for simplicity reasons
	// In the real world these inputs would be sanitized
	char query[100] = "SELECT date, time, message, pgm_name FROM log_1 WHERE (date >= '";
	strcat(query, start_date);
	strcat(query, "' AND date <= '");
	strcat(query, end_date);
	strcat(query, "');");

	// Pull result of query
	query_result = sqlQuery(&mysql, query);

	// If there are zero rows then no data matches the query
	if (mysql_num_rows(query_result) == 0)
	{
		printf("Zero rows fetched\n");
		exit(1);
	}

	// Process the query result and print it out to stdout
	processQuery(query_result, fields);
}

// Fuction to initialize the mysql library
// Throws an error and exits if either of the mysql init functions fail
void sqlInit(MYSQL *mysql)
{
	if (mysql_library_init(0, NULL, NULL)) 
	{
    	fprintf(stderr, "could not initialize MySQL client library\n");
    	exit(1);
  	} 

  	mysql_init(mysql);
  	if (mysql == NULL)
  	{
  		fprintf(stderr, "mysql_init() failed\n");
  		exit(1);
  	}
}

// Function to connect to the mysql server database specified by the
// global defines IP, USER, PASSWORD, and DB
void sqlConnect(MYSQL *mysql)
{
	if (mysql_real_connect(mysql, IP, USER, PASSWORD, DB, 0, NULL, 0) == NULL)
	{
    	fprintf(stderr, "Failed to connect to database\n");
    	exit(1);
	}
}

// Function that runs the specified query on the assumed to be
// already connected and initialized database
// Returns the result of the query
// or exits if the query fails with an error
MYSQL_RES *sqlQuery(MYSQL *mysql, char *query)
{
	mysql_query(mysql, query);
	MYSQL_RES *result = mysql_store_result(mysql);

	if (result == NULL)
	{
		fprintf(stderr, "Query failed with error\n");
		exit(1);
	}

	return result;
}

// This function takes the arguments and parses them into
// our start_date and end_date variables
// the arguments can be in any order, but there
// must be a "-s" argument and a "-e" argument
int parseArgs(char* start_date, char* end_date, int argc, char* argv[])
{
	for (int i = 1; i < argc; i += 2)
	{
		if (strcmp(argv[i], "-s") == 0)
		{
			strcpy(start_date, argv[i + 1]);
		}
		else if (strcmp(argv[i], "-e") == 0)
		{
			strcpy(end_date, argv[i + 1]);
		}
		else
		{
			printf("usage: ./database -s <START DATE> -e <END DATE>\n");
			exit(1);
		}
	}
}

// This function processes the query result and prints
// the data to stdout
// First prints a row with the field names
// and then prints the data as it's read
void processQuery(MYSQL_RES *query_result, char fields[][100])
{
	for (int i = 0; i < 4; i++)
	{
		printf("%s    ", fields[i]);
	}

	printf("\n");

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(query_result)))
	{
		for (int i = 0; i < 4; i++)
		{
			printf("%s    ", row[i] ? row[i] : "NULL");
		}
		printf("\n");
	}
}
