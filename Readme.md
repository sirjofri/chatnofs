# chatnofs

A Plan 9 chat application without a filesystem.

## Run

See the following listener script:

	===/rc/bin/service/tcp9876
	#!/bin/rc
	tlssrv -a /path/to/chatnofs -m /path/to/motd -c /path/to/chanlist -b /path/to/chatdir

The user can join without any client side software, only `tlsclient` is needed:

	tlsclient -a tcp!server!9876

Connections are authenticated, users need accounts on the server and the connection secret in their factotum.

Command line arguments to chatnofs are:

- **-m motd**: Message of the day file (default: none).
- **-c chan**: Channel list file. Similar to MOTD, but you can list your channels there (default: none).
- **-b dir**: Directory that contains channels (default: /mnt/chat).
- **-s chat**: Initial chat (default: lobby).

All accessible channel files need read and write permissions for the authenticated users!
For example, if a certain chat has only read/write permissions for group `chat`, only users of group `chat` will be able to join the chat!
There is currently no read-only join.

## Commands

Commands start with a carriage return character (`\r`, can be easily typed with Ctrl+m).

Currently implemented are:

- **j channel**: join other channels.
- **q**: send quit message and quit.
- **m**: read MOTD.
- **c**: read channel list.

Closing the connection (Ctrl+d) also quits, but without sending a quit message.
