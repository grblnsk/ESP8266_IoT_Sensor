
bmp085 = require "bmp085"

function meas()
bmp085.init(2, 1)
p = bmp085.getUT(false)
print("D0.1C")
print("temperature:"..p)
p = bmp085.getUP(0)
print("pressure:"..p)
bmp085 = nil
package.loaded["bmp085"]=nil
end
tmr.delay(10)
