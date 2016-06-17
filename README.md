This is a project for kernel source code analysis.

After downloading the source code:
    1. sh kemu.sh
    2. . script/setup-env.sh
    3. make
    4. make boot (make mboot)

For single host emulation you just need to "make boot".
For multi-host emulation you just need to "make mboot", this will create two host and a router. And the topoloy will be:
    ---------             --------            -------
    | host1 |------------>|router|<-----------|host2|
    ---------             --------            -------

You can telnet to the mulated host from you local host. This will make the terminal looks better.
