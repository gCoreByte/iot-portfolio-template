#!/usr/bin/env ruby
# frozen_string_literal: true

require "paho-mqtt"

GATEWAY = "iotgateway"

# Set up MQTT client
client = PahoMqtt::Client.new(persistent: true)
client.connect("localhost", 1883)

# Topic helpers
temp_topic = "#{GATEWAY}/temp-measure/temp1"

# Subscribe to temperature sensor
client.subscribe([temp_topic, 0])

# Handle incoming temperature messages
client.on_message do |message|
  msg = message.payload

  t = Integer(msg)
  ac_on = t > 20
  puts "temperature: #{t}, AC on: #{ac_on}"
end

# Keep running
loop do
  sleep 0.5
end
