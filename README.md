# Chat Client
# Summary
  - This is a chat client designed in Qt
  - Features:
    -  Join any server running on any IP/Port
    -  Login/Register accounts
    -  Supports multiple clients
    -  Send smileys (Smileys are added manually in the client project)

# Instructions
##Setup
  - Clone the repository:
    - `git clone https://bitbucket.org/dhillonsh/chat-client <targetDirectory>`
    - `cd <targetDirectory>`
  - `cd` into the `server` directory and execute:
    - `qmake server.pro`
    - `make`
	- This must be running in order for clients to be able to connect
  - `cd` into the  `client` directory and execute:
    - `qmake client.pro`
    - `make`
      - This is what the client uses to connect to the server (the GUI)

##Running
  - Begin by running the server
    - Navigate into the server directory via: `cd server`
    - Run the server: `./server`
    - You will then be prompted to enter a port, this is the port the clients will use when attempting to connect (enter any available port) (example: 11000)
    - You should then see the following prompts which will indicate that the server is running properly:
        - `Listening on port 11000`
        - `Successfully connected to database`
  - Now run the actual chat client GUI
    - You will need to navigate into the client directory and run the `client` application
    - After the GUI opens, you will see a field to enter in the IP and PORT of wherever the server is located
        - If you are running the server and client on the same machine, then the IP would be the localhost IP: 127.0.0.1
        - The port is the port you entered when you ran the `./server` file (11000 in our example)
    - Press `connect` and you should then see the Login/Register page
        - If you do not have an account already, then use the bottom section to register a new account and press `register`
        - Else, login using valid credentials
    - You will then see the chatbox main page
        - On the middle of the page will be the messages that have been sent/received
        - On the right side is the userlist which will display all the users currently in the group and any new users that join
          - The group name is on top of the user list on the right side
          - Under the user list is a buttom called `Join Room`
            - When clicking this, you will navigate to a page that will display a text box. Enter in the group name you wish to join and press `Join Room`
            - Note: You will only see users and messages from within the same group
         - Underneath the chatbox is the input field where you can type a message to be sent, and press `Send`
         - Underneath the input field is a small text box that lists all of the possible emojis
           - To send an emoji in the chat, type in one of the emojis (for example: (smile)) and press `Send`, an example message would be:
               - I feel very happy (happy).
           - Note: you must include the paranthesis when sending emojis
         - On the bottom right is a `Logout` button which will send you back to the server select page and log you out of the chat
  

# Project Updates:
  - 2/20/17 Harpreet - Working on the server portion of the chat application 
  - 2/24/17 Harpreet - Worked on the client portion of the chat application
  - 2/24/17 Jasmit - Working on the GUI portion of the chat on Visual studio windows forms.
  - 2/30/17 Harpreet and Jasmit - Finishing up on the server and client
  - 3/2/17 Harpreet and Jasmit - Re-doing the GUI in QT because threads weren't possible in windows forms. 
  - 3/4/17 Harpreet - Changing GUI to use Signals instead of threads.
  - 3/7/17 Harpreet and Jasmit - Fixing GUI Continued
  - 3/13/17 Harpreet - Adding Login for user authentication
  - 3/14/17 Harpreet and Jasmit - Finishing up login features and cleaning up code 
  - 3/15/17 Harpreet and Jasmit - Added smileys