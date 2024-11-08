/*
 * MailClock.java 11/21/02 by Tom Igoe
 * 
 * Based on:
 * @(#)monitor.java 1.7 01/05/23
 *
 * Copyright 1996-2000 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 * - Redistribution in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND
 * ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES OR LIABILITIES
 * SUFFERED BY LICENSEE AS A RESULT OF  OR RELATING TO USE, MODIFICATION
 * OR DISTRIBUTION OF THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR
 * FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE
 * DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY,
 * ARISING OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * You acknowledge that Software is not designed, licensed or intended
 * for use in the design, construction, operation or maintenance of any
 * nuclear facility.
 */

import java.util.*;
import java.io.*;
import javax.mail.*;
import javax.mail.event.*;
import javax.activation.*;

/* Monitors given mailbox for new mail */

public class MailClock {
  
     public static void main(String argv[]) {
         String[] acct1 = {"somewhere.com","someone","passwd","INBOX"};
         String[] acct2 = {"somewhereElse.com","someoneElse","otherPasswd","INBOX"};
         AccountMonitor firstAcct;
         AccountMonitor secondAcct;
         int i = 0;
      
      BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));  

        if (argv.length != 1) {
           System.out.println("Usage: MailClock ");
           System.exit(1);
        }
        // instantiate UDP sender object:
        SendSiteplayerUDP sender = new SendSiteplayerUDP(26482);
        
        //instantiate account monitors:
        firstAcct = new AccountMonitor(acct1);
        secondAcct = new AccountMonitor(acct2);
                    
        firstAcct.CheckExistingMessages(sender);
        secondAcct.CheckExistingMessages(sender);
        firstAcct.KeepListening(sender);
        secondAcct.KeepListening(sender);
        
       // Check mail once in "freq" MILLIseconds
        int freq = Integer.parseInt(argv[0]);
        while (true) {
            try {
            Thread.sleep(freq); // sleep for freq milliseconds
            // This is to force the IMAP server to send us
            // EXISTS notifications. 
             int numMsgs;
            if (firstAcct.folder.isOpen()) {
               numMsgs = firstAcct.folder.getMessageCount();
            } else {
                 firstAcct.folder.open(Folder.READ_WRITE);
                 numMsgs = firstAcct.folder.getMessageCount();            
            }
            if (secondAcct.folder.isOpen()) {
               numMsgs = firstAcct.folder.getMessageCount();
            } else {
                 secondAcct.folder.open(Folder.READ_WRITE);
                 numMsgs = secondAcct.folder.getMessageCount();            
            }
            
            }catch (Exception ex) {
	    ex.printStackTrace();
	}
        }
    }
}
// Class AccountMonitor:

import java.util.*;
import java.io.*;
import javax.mail.*;
import javax.mail.event.*;
import javax.activation.*;


public class AccountMonitor {

// new bytes of mail that have come in   
int newBytes = 0; 
   
// messages retrieved
Message[] msgs;  

// Mail folder to open 
 Folder folder;
 
   public AccountMonitor(String argv[]) {
            try {
            Properties props = System.getProperties();
            
// Get a Session object
            Session session = Session.getDefaultInstance(props, null);
                 
           // Get a Store object
           Store store = session.getStore("imap");

           // Connect
           store.connect(argv[0], argv[1], argv[2]);

           // Open a Folder
           folder = store.getFolder(argv[3]);
           if (folder == null || !folder.exists()) {
               System.out.println("Invalid folder");
               System.exit(1);
           }

            folder.open(Folder.READ_WRITE);
            msgs = folder.getMessages();
             } catch (Exception ex) {
            ex.printStackTrace();
            }
            
    }
   
   public void CheckExistingMessages (SendSiteplayerUDP sender) {
              // parse the existing messages:
            try {
            for (int i = 0; i < msgs.length; i++) {
                Message m = msgs[i];
                String from = m.getFrom()[0].toString();
                int length = m.getSize();   
                newBytes = newBytes + length;
                String subject = m.getSubject();
            }

// IP address of siteplayer goes here: 
            sender.sendData("xxx.xxx.xxx.xxx", newBytes);
            newBytes = 0;
             } catch (Exception ex) {
            ex.printStackTrace();
            }
   
   }
    public void KeepListening (final SendSiteplayerUDP sender){
                // Add messageCountListener to listen for new messages
        
        try {
        	folder.addMessageCountListener(new MessageCountAdapter() {
                public void messagesAdded(MessageCountEvent ev) {   
                    int newBytes = 0;
                    Message[] msgs = ev.getMessages();

                    // Get info from the new messages
                    for (int i = 0; i < msgs.length; i++) {
                        try {       
                             int length = msgs[i].getSize();
                             newBytes = newBytes + length;
                         } catch (MessagingException mex) {
                            mex.printStackTrace();
                        }
                    }
// IP address of siteplayer goes here: 
                    sender.sendData("xxx.xxx.xxx.xxx", newBytes);
                    newBytes = 0;                               
                }
            });
            } catch (Exception ex) {
            ex.printStackTrace();
            }   
    }
    }
/*
    SendSiteplayerUDP class
    
    Sends a UDP packet formatted for reception by the siteplayer.  
    Specifically, sends one byte only to siteplayer's memory  address FF19h, the COM port.
            
*/


import java.io.*;
import java.lang.*;
import java.util.*;
import java.text.*;
import java.net.*;

public class SendSiteplayerUDP {

    int PORT;                               // the port number we'll send on
    DatagramSocket senderSocket = null;     // the socket we'll open
    DatagramPacket packetToSend = null;     // the packet we'll send
   
   /*
    * Constructor method:
    */
   
    public SendSiteplayerUDP (int portnum) {
        try {
           // open the sending socket:
           senderSocket = new DatagramSocket();
           PORT = portnum;
        } catch (SocketException exp) {System.out.println("couldn't make a socket" + exp);}
    }   

    /*
     *  Socket closer method:
     */
    
    public void closePort() {
        // close the socket:
        senderSocket.close();
    }
    
    /*
     *  Sender method (this version takes a String as the address) 
     */
    
    public void sendData(String IPAddr, int dataInt) {
        
        InetAddress address = null;     // address we'll send to
        int myDataInt = dataInt;
          // get the address from the incoming parameters:
        try {
            address = InetAddress.getByName(IPAddr); 
        } catch (UnknownHostException uhe) {}
        
        // set up a byte array to hold the packet's bytes:
        byte[] dataBuffer = new byte[10];
        
        // put bytes of the int into bytes 4 - 7 of dataArray:
        byte i;
        for (i = 4; i < 7; i++) {
            dataBuffer[i] = (byte)(myDataInt % 256);
            myDataInt = (myDataInt - dataBuffer[i])/256;
        }
        
        // assemble the header and tail of the packet:
        dataBuffer[0] = 0x04;          // number of bytes to send
        dataBuffer[1] = (byte) 0xFB;   // complement of number of bytes to send
        dataBuffer[2] = 0x19;          // low byte of address of COM
        dataBuffer[3] = (byte) 0xFF;   // high byte of address of COM
        dataBuffer[8] = 0x00;          // udp must end with two zero bytes    
        dataBuffer[9] = 0x00;          // udp must end with two zero bytes
 
        packetToSend = new DatagramPacket(dataBuffer, dataBuffer.length, address, PORT);
        // send the packet out the socket:
        try {
            senderSocket.send(packetToSend);
        } catch (IOException expe) {System.out.println(expe);}

       }       
}