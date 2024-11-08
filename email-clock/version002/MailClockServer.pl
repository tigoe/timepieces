#!/usr/bin/perl

use Net::POP3;
$userFile = 'accounts.txt';
$kilobytes = 0;
$totalMsgs = 0;

open(USERS, $userFile )|| die(print "none\0");
	
while(<USERS>){		
	chop($_);
	# get the username, host name, and password
	($username, $host, $password) = split(/\t/, $_);
	
	if (defined($username) && defined($host) && defined($password)) {
		# open a connection to the mail host with the user/pwd info:
    	$pop = Net::POP3->new($host);
    	# check the number of new messages:
    	$numMsgs = $pop->login($username, $password);
    	
    	if ($numMsgs eq "0E0") {
    		# no messages have come in
    	} else {
    		# get the byte count of new messages, add it to the total:
		    ($new, $size) = $pop->popstat();
	   		$kilobytes = $kilobytes + $size;
	    	$totalMsgs = $totalMsgs + $new;	    
    	}
    	# quit the server:
    	$pop->quit;		
	}
}
close(USERS);
# convert bytes to KB:
$kilobytes = $kilobytes/1024;
# Print the result:
print "<KB:";
printf("%.0f", $kilobytes);
print(">\n\n\0");
