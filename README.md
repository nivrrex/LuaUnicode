LuaUnicode
==========
ANSI <-> UNICODE <-> UTF-8的lua扩展

How to use
==========
-- 假定dll文件位于C:\Unicode.dll
-- 假定中文路径的文件名为C:\哈哈.xls
-- 假定C:\Unicode.csv为一个UTF-16LE编码文件

```lua
require "luacom"
local unicode = package.loadlib("C:\\Unicode.dll","luaopen_Unicode")

if(unicode)then
    unicode()
    print("OK!")
else
    print("Not found!")
    -- Error
end

a2u =Unicode.a2u
u2a =Unicode.u2a
u2u8 =Unicode.u2u8
u82u =Unicode.u82u
u82a =Unicode.u82a
a2u8 =Unicode.a2u8
getallfilewc = Unicode.getallfilewc
getallfilews = Unicode.getallfilews
getfilesizew= Unicode.getfilesizew

print(getfilesizew("C:\\Unicode.csv"))
local x = os.clock()
getallfilewc("C:\\Unicode.csv")
print(string.format("elapsed time: %.2f\n",os.clock() - x ))

local x = os.clock()
getallfilews("C:\\Unicode.csv")
print(string.format("elapsed time: %.2f\n",os.clock() - x ))
print(a2u8("哈哈.xls"))

local oExcel = luacom.CreateObject("Excel.Application")
if oExcel == nilthen error("Objectis not create") end
    oExcel.Visible = 1
    oExcel.WorkBooks:Open(a2u8("C:\\哈哈.xls"),nil,0)
    oExcel.ActiveWorkbook.Sheets(1):Select()
```