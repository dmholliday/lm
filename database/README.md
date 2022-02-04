# lm-database

This program will connect to a MySQL database named "user_data" at the default IP address specified below and print the contents of the table "log_1" to stdout with date entries between the start and end dates specified in the command line arguments below (inclusive).

Both arguments must be included, but they can be in any order.

```
usage: ./database -s <START DATE> -e <END DATE>
```

Default IP: 192.168.145.68

Default Port: 3306
  
Default database name: user_data
  
Default table name: log_1

Default user name: process_user

Default password: secret_password
