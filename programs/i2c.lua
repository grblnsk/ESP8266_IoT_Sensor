bmp085.init(1, 2)
local p = bmp085.pressure()
print(string.format("Pressure: %s.%s mbar", p / 100, p % 100))
local t = bmp085.temperature()
print(string.format("Temperature: %s.%s degrees C", t / 10, t % 10))