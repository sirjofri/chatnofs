# chatnofs

A Plan 9 chat application without a filesystem.

## Run

See the following listener script:

	===/rc/bin/service/tcp9876
	#!/bin/rc
	tlssrv -a /path/to/chatnofs -m /path/to/motd -b /path/to/chatdir

The user can join without any client side software, only `tlsclient` is needed:

	tlsclient -a tcp!server!9876

Connections are authenticated, users need accounts on the server and the connection secret in their factotum.

## Commands

Commands start with a carriage return character (`\r`, can be easily typed with Ctrl+m).

Currently implemented are:

- **j channel**: join other channels.
- **q**: send quit message and quit.
- **m**: read MOTD.

Closing the connection (Ctrl+d) also quits, but without sending a quit message.
