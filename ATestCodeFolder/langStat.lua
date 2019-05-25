--[[
/*************************************************************
* Date : 2013-05-04
* Author : FQS
* Desc : 搜索luapages下所有的脚本文件，
		统计出langxx.lua中无用词条--待删除，删除工具的输入，
		统计出langxx.lua中缺失词条--待添加，俄文翻译要求。
**************************************************************/
]]

require'lfs'

-- 待统计的词条文件路径
local langFiles = {
	"langen.lua",
	"langcn.lua"
}

-- 在脚本使用的词条ID
local usedLangs = {}

-- 统计luapages文件夹下脚本使用的词条
function statUsedLangs (scriptFile)
	-- 遍历文件每一行
     for line in io.lines(scriptFile) do
		--print("line = " .. line )

		-- 词条引用格式 &?xxx;
		 for langID in string.gmatch(line, "&?(%w+);") do
		   table.insert(usedLangs, langID)
		   --print("langID " .. langID .. " used in format &?xxx;")
		 end
		-- 词条引用格式 lang.xxx
		 for langID in string.gmatch(line, "lang%.(%w+)") do
		   table.insert(usedLangs, langID)
		   --print("langID " .. langID .. " used in format lang.xxx")
		 end
     end
end

-- 统计luapages文件夹及其子文件夹
function statDirectory(dir)
	for subItem in lfs.dir(dir) do
		if subItem ~= "." and subItem ~= ".." then
			local f = dir.."\\"..subItem

			--print ("\t=> "..f.." <=")

			local attr = lfs.attributes (f)
			--print ("attr.mode="..attr.mode)
			--assert (type(attr) == "table")

			if attr.mode == "directory" then
				statDirectory(f)
			else
				statUsedLangs (f)
			end
		end
	end
end

-- 统计词条
statDirectory(".\\luapages")

print("All used langs in Scripts is below:")
for k,v in pairs(usedLangs) do
	print("usedLang"..k.."="..v);
end

-- 对langxx.lua，统计出多余和缺失词条
for k,v in pairs(langFiles) do
	-- 加载词条文件
	local langPath = ".\\luapages\\config\\"
	local langFile = langPath..v;
	dofile(langFile)

	-- 将统计结果写入到langxx.lua对应文件langxx_stat.txt
	local statFile = io.open(langFile .."_stat.txt", "w+")
	statFile:write("##stat reslut at time(" .. os.date() .. ")\n")

	-- 统计冗余词条
	local langsDel = {}
	for langID, langVal in pairs(lang) do
		local usedflag = "NO";
		for _,usedLang in pairs(usedLangs) do
			if usedLang == langID then
				usedflag = "YES";
			end
		end

		if usedflag == "NO" then
			table.insert(langsDel, langID);
		end
	end

	print("All no used langs in langxx.lua is below:")
	statFile:write("\n--All no used langs in langxx.lua is below:\n")
	for k,v in pairs(langsDel) do
		print("langDel"..k.."="..v);
		statFile:write("langDel"..k.."="..v.."\n");
	end

	-- 统计缺失词条
	local langsMiss = {}
	for _, usedLang in pairs(usedLangs) do
		local existflag = "NO"
		if lang[usedLang] then
			existflag = "YES"
		end

		if existflag == "NO" then
			table.insert(langsMiss, usedLang);
		end
	end

	print("All missed langs in langxx.lua is below:")
	statFile:write("\n--All missed langs in langxx.lua is below:\n")
	for k,v in pairs(langsMiss) do
		print("langDel"..k.."="..v);
		statFile:write("langMiss"..k.."="..v.."\n");
	end

	io.close(statFile)
end

