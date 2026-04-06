#!/usr/bin/env ruby
# frozen_string_literal: true

require "paho-mqtt"

GATEWAY = "iotgateway"

# Set up MQTT client
client = PahoMqtt::Client.new
client.connect("localhost", 1883)

# Topic helpers
switch_topic = "#{GATEWAY}/switch/r1/set"
temp_topic = "#{GATEWAY}/temp-measure/temp1"

# Subscribe to temperature sensor
client.subscribe([temp_topic, 0])

# Handle incoming temperature messages
client.on_message do |message|
  next unless message.topic == temp_topic

  msg = message.payload
  puts "received: [temp] #{msg}"

  begin
    t = Integer(msg)
  rescue ArgumentError
    next
  end

  if t >= 25
    client.publish(switch_topic, "on", retain: false)
    puts "sending: [r1] on"
  else
    client.publish(switch_topic, "off", retain: false)
    puts "sending: [r1] off"
  end
end

# Keep running
loop do
  sleep 0.5
end
