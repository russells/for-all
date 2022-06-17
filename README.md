# for-all

Do something on a list of hosts. "For all of these, do that."

Re-implementation (in C) of my on-all (bash), which was a fork of my
on-all-workstations.

## Examples

- Run the hostname command on every workstation in your 'all' list:
  ```
  for-all -- hostname
  ```

- Install the foo package on two workstations:
  ```
  for-all -u root ws1 ws2 -- yum install -y foo
  ```

- Reboot all workstations in the office.  "office" is a file containing a
  list of workstations:
  ```
  for-all -u root -H office -- shutdown -r now
  ```
  Alternatively:
  ```
  for-all -H office -- sudo shutdown -r now
  ```

- Remove apackage from all except the important workstations.  all and
  important are files containing lists of workstations.
  ```
  for-all -u root -N important -- yum erase -y apackage
  ```

- Look for all foo-client* packages on the remote-office workstations: The
  ```\|``` makes the pipe and grep run on the remote office workstation, not
  locally:
  ```
  for-all -W remote-office -- rpm -qa \| grep foo-client
  ```

- List the mozilla files and directories in /tmp: The ```\*``` makes the wildcard
  expand on the remote workstation, not locally:
  ```
  for-all -u root -- ls -ld /tmp/mozilla\*
  ```

## Hosts and lists

- Hosts can be specified on the command line:
  ```
  for-all host1 host2 host2 -- dosomething
  ```

- Hosts can be specified in a file. If ```hostlist``` contains:
  ```
  host1
  host2
  host3
  ```
  then this command:
  ```
  for-all -H hostlist -- dosomething
  ```
  is equivalent to the command above.

- If not hosts or host lists are specified, for-all uses a default host
  list called ```all```.

- Host lists live in either the current directory, or in
  ```~/etc/for-all```. The default host list is ```~/etc/for-all/all```.

- Host lists can contain blank lines and comments. eg
  ```
  # Perth workstations.
  ws1
  ws2

  # Melbourne workstations.
  mel-ws5
  mel-ws6    # Test workstation.
  ```

## Commands

- A ```--``` must be used to separate for-all arguments from the command to
  run on each host.

- ssh is used to run remote commands.

- Shell special characters that are intended for the remote host must be
  escaped or quoted on the for-all command line. See the examples above.

-- Russell Steicke
