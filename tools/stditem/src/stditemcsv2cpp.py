import os
import re
import sys
import csv

def get_item_type(item_dict):
    if item_dict['stdmode'] == 0:
        return '恢复药水'
    elif item_dict['stdmode'] in [1, 40]:
        return '肉'
    elif item_dict['stdmode'] == 2:
        return '道具'
    elif item_dict['stdmode'] == 3:
        if item_dict['shape'] in [1, 2, 3, 5]:
            return '传送卷轴'
        elif item_dict['shape'] in [4, 9, 10]:
            return '功能药水'
        elif item_dict['shape'] == 11:
            return '道具'
        elif item_dict['shape'] == 12:
            return '强效药水'
        else:
            return '道具'
    elif item_dict['stdmode'] == 4:
        return '技能书'
    elif item_dict['stdmode'] in [5, 6]:
        return '武器'
    elif item_dict['stdmode'] in [10, 11]:
        return '衣服'
    elif item_dict['stdmode'] == 15:
        return '头盔'
    elif item_dict['stdmode'] in [19, 20, 21]:
        return '项链'
    elif item_dict['stdmode'] in [22, 23]:
        return '戒指'
    elif item_dict['stdmode'] in [24, 26]:
        return '手镯'
    elif item_dict['stdmode'] == 25:
        if item_dict['shape'] in [1, 2]:
            return '药粉'
        elif item_dict['shape'] in [5, 7]:
            return '护身符'
        else:
            return '道具'
    elif item_dict['stdmode'] == 30:
        if re.search('勋章', item_dict['name']):
            return '勋章'
        else:
            return '火把'
    elif item_dict['stdmode'] == 31:
        return '道具' # 打捆物品
    elif item_dict['stdmode'] == 41:
        return '金币'
    elif item_dict['stdmode'] == 43:
        return '矿石'
    elif item_dict['stdmode'] == 44:
        return '道具'
    elif item_dict['stdmode'] == 45:
        return '骰子'
    elif item_dict['stdmode'] == 46:
        return '道具'
    elif item_dict['stdmode'] == 47:
        return '道具'
    elif item_dict['stdmode'] == 50:
        return '兑换券'
    return '道具'


def parse_book(item_dict):
    print('    .book')
    print('    {')

    if   item_dict['shape'] == 0: print('        .job = u8"战士",')
    elif item_dict['shape'] == 1: print('        .job = u8"法师",')
    elif item_dict['shape'] == 2: print('        .job = u8"道士",')

    print('        .level = %d,' % item_dict['duramax'])
    print('    },')


def parse_potion(item_dict):
    print('    .potion')
    print('    {')

    if item_dict['ac'] > 0:
        print('        .hp = %d,' % item_dict['ac'])

    if item_dict['mac'] > 0:
        print('        .mp = %d,' % item_dict['mac'])

    if item_dict['shape'] == 0:
        print('        .time = 1,')
    elif item_dict['shape'] == 1:
        pass # add HP/MP immediately
    else:
        pass # unknown types

    print('    },')


def parse_dope(item_dict):
    print('    .dope')
    print('    {')

    if item_dict['ac'] > 0:
        print('        .hp = %d,' % item_dict['ac'])

    if item_dict['mac'] > 0:
        print('        .mp = %d,' % item_dict['mac'])

    if item_dict['dc'] > 0:
        print('        .dc = %d,' % item_dict['dc'])

    if item_dict['mc'] > 0:
        print('        .mdc = %d,' % item_dict['mc'])

    if item_dict['sac'] > 0:
        print('        .sdc = %d,' % item_dict['sac'])

    if item_dict['ac2'] > 0:
        print('        .speed = %d,' % item_dict['ac2'])

    if item_dict['mac2'] > 0:
        print('        .time = %d,' % item_dict['mac2'])

    print('    },')


def parse_equip_require(item_dict):
    if item_dict['needlevel'] > 0:
        print('        .req')
        print('        {')
        if   item_dict['need'] == 0: print('            .level = %d,' % min(60, item_dict['needlevel']))
        elif item_dict['need'] == 1: print('            .dc = %d,' % item_dict['needlevel'])
        elif item_dict['need'] == 2: print('            .mc = %d,' % item_dict['needlevel'])
        elif item_dict['need'] == 3: print('            .sc = %d,' % item_dict['needlevel'])
        else: pass
        print('        },')


def parse_equip_attr_helmet(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['ac2'] > 0:
        item_attr['hit'] = item_dict['ac2']

    if item_dict['ac'] > 0 or item_dict['mac'] != 0:
        item_attr['luckCurse'] = item_dict['ac'] - item_dict['mac']

    if item_dict['mac2'] > 0:
        item_attr['speed'] = -item_dict['mac2']

    if item_dict['source'] > 0:
        item_attr['elem'] = {}
        item_attr['elem']['holy'] = item_dict['source']

    return item_attr


def parse_equip_attr_necklace(item_dict):
    item_attr = {}
    if item_dict['stdmode'] == 19:
        item_attr['mcDodge'] = item_dict['ac2']
        item_attr['luckCurse'] = item_dict['mac'] - item_dict['mac2']
    elif item_dict['stdmode'] == 20:
        item_attr['hit'] = item_dict['ac2']
        item_attr['dcDodge'] = item_dict['mac2']
    elif item_dict['stdmode'] == 21:
        item_attr['speed'] = item_dict['ac'] - item_dict['mac']
        item_attr['hpRecover'] = item_dict['ac2']
        item_attr['mpRecover'] = item_dict['mac2']

    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]
        elif item_dict['mc_type'] == 2:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    return item_attr


def parse_equip_attr_armring(item_dict):
    item_attr = {}
    if item_dict['stdmode'] == 24:
        if item_dict['ac2'] > 0:
            item_attr['hit'] = item_dict['ac2']

        if item_dict['mac2'] > 0:
            item_attr['dcDodge'] = item_dict['mac2']

    elif item_dict['stdmode'] == 26:

        if item_dict['ac'] > 0 or item_dict['ac2'] > 0:
            item_attr['ac'] = [item_dict['ac'], item_dict['ac2']]

        if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
            item_attr['mac'] = [item_dict['mac'], item_dict['mac2']]
    else:
        pass

    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]
        elif item_dict['mc_type'] == 2:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    return item_attr


def parse_equip_attr_ring(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['ac2'] > 0:
        item_attr['hit'] = item_dict['ac2']

    if item_dict['ac'] > 0 or item_dict['mac'] != 0:
        item_attr['luckCurse'] = item_dict['ac'] - item_dict['mac']

    if item_dict['mac2'] > 0:
        item_attr['speed'] = -item_dict['mac2']

    if item_dict['source'] > 0:
        item_attr['elem'] = {}
        item_attr['elem']['holy'] = item_dict['source']

    return item_attr


def parse_equip_attr_medal(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]
        elif item_dict['mc_type'] == 2:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    if item_dict['ac'] > 0 or item_dict['ac2'] > 0:
        item_attr['ac'] = [item_dict['ac'], item_dict['ac2']]

    if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
        item_attr['mac'] = [item_dict['mac'], item_dict['mac2']]

    return item_attr


def parse_equip_attr_cloth(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]
        elif item_dict['mc_type'] == 2:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    if item_dict['ac'] > 0 or item_dict['ac2'] > 0:
        item_attr['ac'] = [item_dict['ac'], item_dict['ac2']]

    if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
        item_attr['mac'] = [item_dict['mac'], item_dict['mac2']]

    return item_attr


def parse_equip_attr_weapon(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]
        elif item_dict['mc_type'] == 2:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    if item_dict['ac2'] > 0:
        item_attr['hit'] = item_dict['ac2']

    if item_dict['ac'] > 0 or item_dict['mac'] != 0:
        item_attr['luckCurse'] = item_dict['ac'] - item_dict['mac']

    if item_dict['mac2'] > 0:
        item_attr['speed'] = -item_dict['mac2']

    if item_dict['source'] > 0:
        item_attr['elem'] = {}
        item_attr['elem']['holy'] = item_dict['source']

    return item_attr


def parse_equip(item_dict):
    item_type = get_item_type(item_dict)
    item_attr = {}

    print('    .equip')
    print('    {')
    print('        .duration = %d,' % max(1, item_dict['duramax'] // 1000))

    if   item_type == '头盔': item_attr = parse_equip_attr_helmet  (item_dict)
    elif item_type == '项链': item_attr = parse_equip_attr_necklace(item_dict)
    elif item_type == '手镯': item_attr = parse_equip_attr_armring (item_dict)
    elif item_type == '戒指': item_attr = parse_equip_attr_ring    (item_dict)
    elif item_type == '勋章': item_attr = parse_equip_attr_medal   (item_dict)
    elif item_type == '衣服': item_attr = parse_equip_attr_cloth   (item_dict)
    elif item_type == '武器': item_attr = parse_equip_attr_weapon  (item_dict)
    else: raise ValueError(item_dict['name'], item_type)

    if 'dc' in item_attr:
        print('        .dc = {%d, %d},' % (item_attr['dc'][0], item_attr['dc'][1]))

    if 'mc' in item_attr:
        print('        .mc = {%d, %d},' % (item_attr['mc'][0], item_attr['mc'][1]))

    if 'sc' in item_attr:
        print('        .sc = {%d, %d},' % (item_attr['sc'][0], item_attr['sc'][1]))

    if 'ac' in item_attr:
        print('        .ac = {%d, %d},' % (item_attr['ac'][0], item_attr['ac'][1]))

    if 'mac' in item_attr:
        print('        .mac = {%d, %d},' % (item_attr['mac'][0], item_attr['mac'][1]))

    if 'hit' in item_attr:
        print('        .hit = %d,' % item_attr['hit'])

    if 'dcDodge' in item_attr:
        print('        .dcDodge = %d,' % item_attr['dcDodge'])

    if 'mcDodge' in item_attr:
        print('        .mcDodge = %d,' % item_attr['mcDodge'])

    if 'speed' in item_attr:
        print('        .speed = %d,' % item_attr['speed'])

    if 'comfort' in item_attr:
        print('        .comfort = %d,' % item_attr['comfort'])

    if 'hpRecover' in item_attr:
        print('        .hpRecover = %d,' % item_attr['hpRecover'])

    if 'mpRecover' in item_attr:
        print('        .mpRecover = %d,' % item_attr['mpRecover'])

    if 'luckCurse' in item_attr:
        print('        .luckCurse = %d,' % item_attr['luckCurse'])

    if 'elem' in item_attr:
        print('        .elem')
        print('        {')

        if 'fire' in item_attr['elem']:
            print('            .fire = %d' % item_attr['elem']['fire'])

        if 'ice' in item_attr['elem']:
            print('            .ice = %d' % item_attr['elem']['ice'])

        if 'light' in item_attr['elem']:
            print('            .light = %d' % item_attr['elem']['light'])

        if 'wind' in item_attr['elem']:
            print('            .wind = %d' % item_attr['elem']['wind'])

        if 'holy' in item_attr['elem']:
            print('            .holy = %d' % item_attr['elem']['holy'])

        if 'dark' in item_attr['elem']:
            print('            .dark = %d' % item_attr['elem']['dark'])

        print('        }')

    if 'load' in item_attr:
        print('        .load')
        print('        {')

        if 'hand' in item_attr['load']:
            print('            .hand = %d' % item_attr['load']['hand'])

        if 'body' in item_attr['load']:
            print('            .body = %d' % item_attr['load']['body'])

        if 'inventory' in item_attr['load']:
            print('            .inventory = %d' % item_attr['load']['inventory'])

        print('        }')

    parse_equip_require(item_dict)
    print('    },')


def parse_item(item_dict):
    item_name = item_dict['name']
    item_type = get_item_type(item_dict)

    print('{   .name = u8"%s",' % item_name)
    print('    .type = u8"%s",' % item_type)
    print('    .weight = %d,' % item_dict['weight'])
    print('    .pkgGfxID = 0X%04X,' % item_dict['looks'])

    if item_type == '恢复药水':
        parse_potion(item_dict)
    elif item_type == '功能药水':
        pass
    elif item_type == '强效药水':
        parse_dope(item_dict)
    elif item_type == '技能书':
        parse_book(item_dict)
    elif item_type in ['头盔', '项链', '手镯', '戒指', '勋章', '衣服', '武器']:
        parse_equip(item_dict)

    print('},')
    print()


def parse_stditem(filename):
    with open(filename, newline='') as csvfile:
        item_reader = csv.reader(csvfile)
        header = None
        item_list = []
        for item_row in item_reader:
            if not header:
                header = item_row
                continue

            item_dict = {}
            for i in range(len(header))[0:-1]:
                if   i == 0: pass
                elif i == 1: item_dict[header[i].lower()] = item_row[i]
                else       : item_dict[header[i].lower()] = int(item_row[i])

            item_list.append(item_dict)

        for item_dict in sorted(item_list, key = lambda item_dict: (item_dict['looks'], item_dict['stdmode'], item_dict['shape'])):
            parse_item(item_dict)


def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: python3 stditemcsv2cpp.py mir2x/readme/sql2csv/King_StdItems.csv")
    parse_stditem(sys.argv[1])


if __name__ == "__main__":
    main()