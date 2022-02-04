# lm-barcode

This program will open a TCP/IP socket to a capable device (in this case an ethernet connected barcode scanner) and print the data the device sends to stdout.

Will attempt to reconnect every 5 seconds indefinitely if a connection is not established or the device closes the connection unexpectedly.

Use the -v command-line argument to turn on verbose mode and see the connection status or no command-line arguments for the normal mode.

```
usage: ./barcode -v (optional)
```

Default IP:   192.168.187.31

Default Port: 51000
