"""
Example of simple consumer. Acks each message as it arrives.
"""
import os
import sys
import krengine

# Detect if we're running in a git repo
from os.path import exists, normpath
if exists(normpath('../pika')):
    sys.path.insert(0, '..')

krengine_libpath = '/home/tiger/krproject/lib'
if os.path.exists(krengine_libpath):
    sys.path.append(krengine_libpath)

from pika.adapters import SelectConnection
from pika.connection import ConnectionParameters

# We use these to hold our connection & channel & engine
connection = None
channel = None
engine = None


def on_connected(connection):
    global channel
    print "demo_receive: Connected to RabbitMQ"
    connection.channel(on_channel_open)


def on_channel_open(channel_):
    global channel
    channel = channel_
    print "demo_receive: Received our Channel"
    channel.queue_declare(queue="krqueue",
                          durable=True,
                          exclusive=False,
                          auto_delete=False,
                          callback=on_queue_declared)


def on_queue_declared(frame):
    print "demo_receive: Queue Declared"
    channel.basic_consume(handle_delivery, queue='krqueue')


def handle_delivery(channel, method_frame, header_frame, body):
    global engine
    print "Basic.Deliver %s delivery-tag %i: %s" %\
          (header_frame.content_type,
           method_frame.delivery_tag,
           body)
    channel.basic_ack(delivery_tag=method_frame.delivery_tag)
    # Call krengine to handle this message
    engine.run(1, 1, body)


if __name__ == '__main__':
    
    # Startup krengine
    engine = krengine.KREngine(shmkey = 74561, threadcnt = 5)
    
    # Connect to RabbitMQ
    host = (len(sys.argv) > 1) and sys.argv[1] or '127.0.0.1'
    connection = SelectConnection(ConnectionParameters(host),
                                  on_connected)
    # Loop until CTRL-C
    try:
        # Start our blocking loop
        connection.ioloop.start()

    except KeyboardInterrupt:

        # Close the connection
        connection.close()
         
        # Shutdown krengine
        engine.shutdown()

        # Loop until the conneciton is closed
        connection.ioloop.start()
