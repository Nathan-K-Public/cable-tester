print('''eMarker decoding script (for Python 3.x)
Accepts CLI & terminal input in "Twinkie" hex format (or Python-delimeted int)

created by Nathan K, Google Nexus TC

Version 1.1p [PUBLIC] - Currently "Cable/Plug" only
TODO: Add UFP/Hub rules, switching, etc''')

##Debug
##ff008041 1c00291a ffffffff ffffffff ffffffff
##Anker (problem)
##ff008041 1c00291a 00000d0b 1ff90000 11082032
##Belkin
##ff008041 1c00050d 00000000 030a0000 11082032
##Belkin active
##ff008041 240020c2 00000000 0003000f 21085838


print()

import sys
import string
import collections
##subfield = collections.namedtuple('Subfield', ['name', 'key', 'bits', 'a','b'])

cliInput=format(" ".join(sys.argv[1:]))

print("Command line arguments: <{}>".format(cliInput))


forcedType='Cable Plug'
###################
## HARDCODED - needs to be entered in menu
##################

def checkHex(s):
    '''Check if a a string is a valid hexadecimal number'''
    try:
        int(field, 16)
    except:
        return False
    return True

def bitStrip(val,a,b):
    '''Strip out a seciton of bits from A to B, inclusive'''
    val = val >> b
    mask = pow(2,a-b+1)-1
    return val & mask; 


##class Subfield(collections.namedtuple('Subfield', [ 'name', 'a','b', 'key', 'bits'])):
##    def __new__(cls, name, a, b, key=0, bits=0):
##        return super(Subfield, cls).__new__(cls, name, a, b, key, bits=a-b+1)

class Subfield:
    def __init__(self, name, a, b, key=0, bits=0):
        self.name = name
        self.a = a
        self.b = b
        self.key = key
        self.bits = a-b+1


##class Rectangle(collections.namedtuple('Rectangle', [ "length", "width", "color"])):
##    def __new__(cls, length, width, color="white"):
##        return super(TemplateContainer, cls).__new__(cls, length, width, color)

vdo = collections.OrderedDict();

vdo['VDM Header']=[
    Subfield('Raw VDM'                                   ,31,0)
    ]




vdo['ID Header']=[
    Subfield('USB Communications Capable as USB Host'     ,31,31),
    Subfield('USB Communications Capable as USB Device'   ,30,30),
    Subfield('Product type (UFP || Cable Plug)'           ,29,27),
    Subfield('Modal Operation Supported'                  ,26,26),
    Subfield('Reserved (zero)'                            ,25,16),
    Subfield('USB Vendor ID (16-bit uint)'                ,15, 0)
    ]

vdo['ID Header'][0].key={
    1:'Capable of enumerating USB devices',
    0:'<default> Cannot enumerate USB devices'
    }
vdo['ID Header'][1].key={
    1:'Capable of enumerating as USB device',
    0:'<default> Cannot enumerate as USB device'
    }

##if productType == 'UFP':
##    vdo['ID Header'][2]={
##        0b000:'Undefined',
##        0b001:'PDUSB Hub',
##        0b010:'PDUSB Peripheral',
##        0b101:'Alternate Mode Adapter (AMA)'
##    }
##elif productType == 'Cable Plug':
##    vdo['ID Header'][2]={
##        0b000:'Undefined',
##        0b011:'Passive Cable',
##        0b100:'Active Cable'
##    }


## KEY SWITCH LOCATED HERE
vdoIDkey2UFP={
    0b000:'Undefined',
    0b001:'PDUSB Hub',
    0b010:'PDUSB Peripheral',
    0b101:'Alternate Mode Adapter (AMA)'
}

vdoIDkey2Cable={
    0b000:'Undefined',
    0b011:'Passive Cable',
    0b100:'Active Cable'
}

vdo['ID Header'][2].key=vdoIDkey2Cable
## Work on a better way to do this later..

vdo['ID Header'][3].key={
    1:'Modal Operation supported',
    0:'<default> Cannot Modal Operate'
    }    



vdo['Cert Stat']=[
    Subfield('XID (32-bit uint)'                          ,31, 0),
    ]

vdo['Product']=[
    Subfield('USB Product ID (dev desc)(16-bit uint)'     ,31,16),
    Subfield('bcdDevice (ver)(dev desc) (16-bit uint)'    ,15, 0),
    ]


## KEY SWITCH LOCATED HERE
##Need to add active cable sometime..... figure out how to slot it in...
vdo['Passive Cable']=[
    Subfield('HW version (VID owner)'               ,31,28),
    Subfield('Firmware version (VID owner)'         ,27,24),
    Subfield('Reserved (zero)'                      ,23,20),
    Subfield('USB Type-C plug t Type-(?)'           ,19,18),
    Subfield('Reserved (zero)'                      ,17,17),
    Subfield('Cable Latency'                        ,16,13),
    Subfield('Cable Termination Type'               ,12,11),
    Subfield('SSTX1 Directionality Support'         ,10,10),
    Subfield('SSTX2 Directionality Support'         , 9, 9),
    Subfield('SSRX1 Directionality Support'         , 8, 8),
    Subfield('SSRX2 Directionality Support'         , 7, 7),
    Subfield('Vbus Current Handling Capability'     , 6, 5),
    Subfield('Vbus through cable'                   , 4, 4),
    Subfield('Reserved (zero)'                      , 3, 3),
    Subfield('USB SuperSpeed Signaling Support'     , 2, 0),
    ]

vdo['Passive Cable'][3].key={
    0b00:'USB Type-A',
    0b01:'USB Type-B',
    0b10:'USB Type-C',
    0b11:'Captive'
    }

##vdo['Passive Cable'][5]={
##    0b0001:'<10ns (~1m)',
##    0b0010:'10ns to 20ns (~2m)',
##    0b0011:'20ns to 30ns (~3m)',
##    0b0100:'30ns to 40ns (~4m)',
##    0b0101:'40ns to 50ns (~5m)',
##    0b0110:'50ns to 60ns (~6m)',
##    0b0111:'60ns to 70ns (~6m)',
##    0b1000:'>70 (>~7m)'
##    }

## KEY SWITCH LOCATED HERE
vdoCablekey5passive={
    0b0001:'<10ns (~1m)',
    0b0010:'10ns to 20ns (~2m)',
    0b0011:'20ns to 30ns (~3m)',
    0b0100:'30ns to 40ns (~4m)',
    0b0101:'40ns to 50ns (~5m)',
    0b0110:'50ns to 60ns (~6m)',
    0b0111:'60ns to 70ns (~6m)',
    0b1000:'>70 (>~7m)'
    }

vdoCablekey5active={
    0b0001:'<10ns (~1m)',
    0b0010:'10ns to 20ns (~2m)',
    0b0011:'20ns to 30ns (~3m)',
    0b0100:'30ns to 40ns (~4m)',
    0b0101:'40ns to 50ns (~5m)',
    0b0110:'50ns to 60ns (~6m)',
    0b0111:'60ns to 70ns (~6m)',
    0b1000:'1000ns (~100m)',
    0b1001:'2000ns (~200m)',
    0b1010:'3000ns (~300m)'
    }

vdo['Passive Cable'][5].key=vdoCablekey5passive
## Work on a better way to do this later..
## Titling it "Passive" makes it look funny later.

vdoCablekey6passive={
    0b00:'Vconn not required (or Cable Plug only supports Discover Identity)',
    0b01:'Vconn required'
    }

vdoCablekey6active={
    0b10:'Vconn required (1 end active, 1 end passive)',
    0b11:'Vconn required (both ends active)'
    }

vdo['Passive Cable'][6].key=vdoCablekey6passive
## Work on a better way to do this later.. <--------------- 11/5/2016

vdo['Passive Cable'][7].key={
    0b00:'Fixed (no significance)',
    0b01:'Configurable (no significance)'
    }

vdo['Passive Cable'][8].key=vdo['Passive Cable'][7].key
vdo['Passive Cable'][9].key=vdo['Passive Cable'][7].key
vdo['Passive Cable'][10].key=vdo['Passive Cable'][7].key

vdo['Passive Cable'][11].key={
    0b01:'3A',
    0b10:'5A'
    }

vdo['Passive Cable'][12].key={
    1:'Yes',
    0:'No'
    }


vdoCablekey13active={
    0b0:'No SOP" controller present',
    0b1:'SOP" controller present'
    }

vdoCablekey13passive={
    0b0:'Reserved (should be 0)'
    }

###vdo['Passive Cable'][13].key=vdoCablekey13active

vdo['Passive Cable'][14].key={
    0b000:'USB 2.0 only, no SuperSpeed support',
    0b001:'USB 3.1 Gen1',
    0b010:'USB 3.1 Gen1 and Gen2'
    }

## NEED TO ADD AMA ADAPTER SIMILARLY





## print(vdo['VDM Header'])

headers=['VDM Header','ID Header VDO','Cert Stat VDO', 'Product VDO',
         'Product Type VDO(0)','Product Type VDO(1)',
         'Product Type VDO(2)','Product Type VDO(3)']



if len(cliInput)!=0:
    var = cliInput
else:
    var = input("Enter eMarker: ")

fields = var.split()
print("you entered <{0}>".format(fields))
print()

##print('{:>{}s}'.format('bojangles', 20))
bail = False;
for i,field in enumerate(fields):
    print()
    print('{}: {}'.format(headers[i],field))
    if checkHex(field) == False:
        print('The value <{}> in index {} is not hex.'.format(field,i))
        bail = True
        continue
    elif int(field,16) > 0xffffffff:
        print('The value <{}> in index {} is too large.'.format(field,i))
        bail = True
        continue

    print('Binary is {:032b}'.format(int(field,16)))
    
    if headers[i]=='ID Header VDO':

        productType = forcedType
        if productType=='UFP':
            print('\nUFP indicated')
            vdo['ID Header'][2].key=vdoIDkey2UFP
        elif productType=='Cable Plug':
            print('\nCable Plug indicated')
            vdo['ID Header'][2].key=vdoIDkey2Cable            
            
            bits = bitStrip(int(field,16),vdo['ID Header'][2].a,vdo['ID Header'][2].b)
            print('Bitstrip said {:b}'.format(bits))

            cableType=vdo['ID Header'][2].key.setdefault(bits)
            if cableType==None:
                print('Warning, unhandled <{}> detected <0b{:b}>'.format(vdo['ID Header'][2].name, bits))
                bail=True
                continue
            elif cableType=='Active Cable':
                print('Active Cable indicated, setting key')
                vdo['Passive Cable'][5].key=vdoCablekey5active
                vdo['Passive Cable'][6].key=vdoCablekey6active
                vdo['Passive Cable'][13].key=vdoCablekey13active
                vdo['Passive Cable'][13].name='SOP" Controller Present'

                vdo1=collections.OrderedDict(('Active Cable' if k == 'Passive Cable' else k, v) for k, v in vdo.items())
                vdo=vdo1
                #Dirty dirty hack to rename the OrderedDict entry
                
            elif cableType=='Passive Cable':
                print('Passive Cable indicated, setting key')
                vdo['Passive Cable'][5].key=vdoCablekey5passive
                vdo['Passive Cable'][6].key=vdoCablekey6passive

        else:
                print('Warning, unhandled <{}> detected <0b{:b}>'.format('Hardcoded product type', productType))
                bail=True
                continue
        

if i+1>8:
    print('There are {} fields, more than 8 max.'.format(i+1))
    bail=True;
    
if bail:
    print('Something went wrong, aborting.')
    sys.exit()
print('\nThere were',i+1,'items')



## Maybe construct an ordered dict on the fly?


for j,(key, vdo_field) in enumerate(vdo.items()):
    #print('\n=====> vdo_field',j, key, vdo_field)
    print('\n[VDO {}/{}] {}'.format(j, headers[j], key))
    
    for k,vdo_subfield in enumerate(vdo_field):
        ##print('\n\t[SUBFIELD {}] {}'.format(k,vdo_subfield.name))
        print('\n\t{1}'.format(k,vdo_subfield.name))
        
        try:
            fields[j]
        except:
            print('\n\nError accessing fields, did you enter enough VDOs?')
            sys.exit()
        
        bits = bitStrip(int(fields[j],16),vdo_subfield.a,vdo_subfield.b)
        ##print('\tBitstrip said {:b}'.format(bits))

        
        keytype=vdo_subfield.key
        if keytype == 0:
            ##print('\tNo decoder key!')
            print('\tHex/no key: {:x}'.format(bits))
        else:
            decoded=vdo_subfield.key.setdefault(bits)
            if decoded == None:
                print('\t<0b{1:0>{2}b}> Warning, unhandled <{0}> detected'.format(vdo_subfield.name,
                                                                                 bits,vdo_subfield.bits))
                for key, keytext in enumerate(vdo_subfield.key.items()):
                    print('\t\t0b{0:>0{2}b}: {1}'.format(key,keytext,vdo_subfield.bits))
            else:
                print('\t<0b{0:>0{1}b}> {2}'.format(bits,vdo_subfield.bits,decoded))



print('End')
