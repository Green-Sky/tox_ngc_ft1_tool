# Tool to send and receive files using Tox NGC FileTransfers
currently on prototype FT1

## Usage

```
$ tox_ngc_ft1_tool
~~~ HELP ~~~
 meta:
 -v version info
 -V verbose
 -h help

 connectivity:
 -G <chat_id>
 -F profile.tox
 -N <self_name> (defaults to 'tox_ngc_ft1_tool')
 will print friend id at startup
 will autoaccept any invite
 if no -F given, will not save profile.
 if profile exists load, otherwise create new

 transfer variant:
 -a id1/sha1_single/sha1_info/sha2_single/sha2_info

 send:
 -f send_this_file.zip

 receive:
 -d dump/everything/in/this/dir
 -D <id/hash> (what to dl)

!!! ADVANCED !!!
 tox:
 -L disable local discovery
 -U disable udp
 -P proxy_host proxy_port
 -p tox_port (bind tox to that port)
```

