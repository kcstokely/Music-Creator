############################################################################################################################################

import math

# converts an integer into a variable-length-quantity

def get_bytes(delta):
    
    if delta == 0:
        
        b = [0]
    
    else:
    
        n = int(math.floor(math.log(delta, 128) + 1))

        b = [128] * (n-1)
        b.append(0)
        b.reverse()

        for i in range(n):
            b[i] += (delta//(128**i)) % 128

        b.reverse()

    return b

############################################################################################################################################

def w_note_on(fp, delta, channel, pitch, velocity):

    # we will need this
    blist = get_bytes(delta)

    # this event happens some time after the previous event
    fp.write(bytearray(blist))
    
    # this event is a note on for a particular channel with pitch / velocity
    fp.write(bytearray([9*16+channel, pitch, velocity]))
    
    return len(blist)+3

############################################################################################################################################

def w_note_off(fp, delta, channel, pitch, velocity):

    # we will need this
    blist = get_bytes(delta)

    # this event happens some time after the previous event
    fp.write(bytearray(blist))
    
    # this event is a note off for a particular channel with pitch / velocity
    fp.write(bytearray([8*16+channel, pitch, velocity]))
    
    return len(blist)+3

############################################################################################################################################

def w_track_end(fp):
    
    # the last event happens at the same time as the last event
    fp.write(bytearray([0]))

    # this last event is the end of the track
    fp.write(bytearray([255, 47, 0]))

    return 4

############################################################################################################################################

def w_track_head(fp, bpm):
    
    # this is a track header
    fp.write(bytearray([77, 84, 114, 107]))

    # this track is some number of bytes long
    fp.write(bytearray([0, 0, 0, 0]))

    # the first event happens at time zero
    fp.write(bytearray([0]))
    
    # this first event is a tempo setting
    fp.write(bytearray([255, 81, 3]))
    
    # here is the tempo info: the number of microseconds in a quarter note (hardlocked at 60 bpm right now)
    fp.write(bytearray([15, 66, 64]))

    # the second event happens at delta t zero
    fp.write(bytearray([0]))

    # this second event is a time signature
    fp.write(bytearray([255, 88, 4]))
    
    # here is the time signature info
    fp.write(bytearray([bpm])) # numerator
    fp.write(bytearray([2]))   # denominator (log base 2)
    fp.write(bytearray([24]))  # number of clocks in a metronome click (there are 24 clocks in a quarter note)
    fp.write(bytearray([8]))   # number of clocks in a 32nd note (8 makes sense, but you can fuck with space-time here)

    return 15

############################################################################################################################################

def w_track_fix(fp, loc, total):

    b = [None] * 4
    
    for i in range(4):
        b[i] = (total//(256**i)) % 256
    
    b.reverse()
    
    fp.seek(loc+4)
    fp.write(bytearray(b))
    fp.seek(0, 2)

    return 0
        
############################################################################################################################################
  
def w_file_head(fp, ppqn):

    # this is a file header
    fp.write(bytearray([77, 84, 104, 100]))

    # this file header is 6 bytes long
    fp.write(bytearray([0, 0, 0, 6]))

    # this file has multiple (synchronous) tracks
    fp.write(bytearray([0, 1]))
        
    # this file has two tracks in fact
    fp.write(bytearray([0, 2]))

    # this file has ppqn divs in a quarter note   
    fp.write(bytearray([0, ppqn]))

    return 14
        
############################################################################################################################################














