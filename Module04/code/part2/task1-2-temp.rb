#!/usr/bin/env ruby
# frozen_string_literal: true

require "paho-mqtt"

GATEWAY = "iotgateway"

# Set up MQTT client
client = PahoMqtt::Client.new(persistent: true)
client.connect("localhost", 1883)

# Topic helpers
temp_topic = "#{GATEWAY}/temp-measure/temp1"

client.publish(temp_topic, "1", retain: false)

temps = ((15..25).to_a + (24..16).step(-1).to_a)
cycled_temps = temps.cycle
loop do
  t = cycled_temps.next
  puts "sending: [temp] #{t}"
  client.publish(temp_topic, t.to_s, retain: false)
  sleep 0.5
end
