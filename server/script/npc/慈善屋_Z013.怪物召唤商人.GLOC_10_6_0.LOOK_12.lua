processNPCEvent = {}
do
    local monsterID = 1
    local monsterNameStrList = {}

    while(true) do
        local monsterName = getMonsterName(monsterID)
        if not hasChar(monsterName) then
            break
        end

        local suffixDigits = string.match(monsterName, '^.-(%d+)$')
        if suffixDigits == nil then
            if monsterName ~= '未知' then
                local tagName = string.format('goto_tag_%d', monsterID)
                monsterNameStrList[#monsterNameStrList + 1] = string.format([[<event id="%s" wrap="false">%s，</event>]], tagName, monsterName)

                processNPCEvent[tagName] = function(uid, value)
                    addMonster(monsterName)
                end
            end
        end
        monsterID = monsterID + 1
    end

    processNPCEvent[SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>客官%s你好我是%s，我可以召唤所有的怪物哦！<emoji id="0"/></par>
                <par></par>
                <par align="justify">%s</par>
                <par></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), table.concat(monsterNameStrList), SYS_NPCDONE)
    end
end
