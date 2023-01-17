# Tool to send and receive files using Tox NGC FileTransfers
currently on prototype FT1

## Example

### sender (seeder)
$ tox_ngc_ft1_tool -F sender.tox -G <ngc_chat_id> -a sha1_info -f file_to_transfer

this will print this tools tox_id and the generated info_hash

### receivers (leechers)
$ tox_ngc_ft1_tool -F sender.tox -G <ngc_chat_id> -a sha1_info -D <info_hash>

this will first download the info using the info_hash and then all the file chunks listed in the info
while simultaneously resharing allready downloaded chunks and info (swarming)

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

 FT1:
 --ft_ack_per_packet
 --ft_init_retry_timeout_after
 --ft_sending_resend_without_ack_after
 --ft_sending_give_up_after
 --ft_packet_window_size
```

