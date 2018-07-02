				$Id: README,v 1.3 2000/01/03 08:56:55 js Exp $


UQWK
                       Copyright     1993-1994,             steve belczyk
                       Modifications 1996, 1997, 1998, 1999,
                                     2000                   jeroen scheerder


uqwk is a program which collects all a user's unread mail or news
and formats it into a packet for offline reading.  QWK, Simple
Offline Usenet Packet (SOUP), and ZipNews packet formats are supported.
See the manual page for sommand line options, environment variables and
examples.

Uqwk also accepts reply packets, so replies can be mailed or posted,
depending whether the message is marked private (email) or public (news).

Uqwk also supports a small offline command language, so the contents
of the user's .newsrc file can be viewed and manipulated offline.

0.  Install uqwk if not installed already; see the INSTALL file.

1.  Now you can try it.  Login as a normal user who has some mail.
    First, tell uqwk to print its options: issue

	uqwk -p

    Check that the output reflects your choices in the makefile.
    Next, issue:

	uqwk +r +m

    (The "+r" stops uqwk from clearing the user's mail spool file.)
    This should create a number of *.MSG files in your home directory,
    along with a file named AREAS.  These are the files the offline
    SOUP reader will need.  (Some readers expect these files to have been
    archived using an archiver like zip, lharc, or arj.  You may need
    to obtain Unix versions of these archivers.)

2a. If you will not be using uqwk in QWK mode (hurray!), you need not
    even  bother reading [2b]; advance to [3].
2b. If you are using uqwk to create QWK packets (please *don't*, really,
    since the QWK format does not allow storing Usenet messages
    correctly): QWK packets contain the name, location, etc., of the BBS
    from which messages are being downloaded.  If you plan to use the
    QWK format (please, please, please don't!), you should configure
    this information. The best way is probably to use environment
    variables.  If you are using a Bourne shell, you should add
    something like this to /etc/profile or .profile:

        UQ_BBS_NAME="My Super BBS"
	UQ_BBS_CITY="Somewhere, PA"
	UQ_BBS_PHONE="+1 215 555 1212"
	UQ_BBS_SYSOP="Joe Shmoe"
	UQ_BBS_ID="0,MYBBS"
	export UQ_BBS_NAME UQ_BBS_CITY UQ_BBS_PHONE
	export UQ_BBS_SYSOP UQ_BBS_ID

    If you use a C type shell, try something like this in your
    .cshrc or .login:

        setenv UQ_BBS_NAME "My Super BBS"
	setenv UQ_BBS_CITY "Somewhere, PA"
	setenv UQ_BBS_PHONE "+1 215 555 1212"
	setenv UQ_BBS_SYSOP "Joe Shmoe"
	setenv UQ_BBS_ID "0,MYBBS"

    In both cases, the last entry, the "BBS ID", is the most important.
    It always consists of an integer, a comma, and a string less than
    nine characters, with no intervening spaces.  The string will be
    used to identify reply packets.

3.  Now would be a good time for reading the man page ("man uqwk")
    and the Frequently Asked Questions list ("more FAQ").
