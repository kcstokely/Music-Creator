############################################################################################################################################

import numpy as np

import music_write_ as mid

def norm(vec):
    return [ float(v)/float(sum(vec)) for v in vec ]

############################################################################################################################################

ppqn = 192

# song parameters

song_measures = 4*4*4

beats_per_measure  = 4

chord_measures_min = 1
chord_measures_max = 6

# bulk parameters

b_num       = 10    # number of bulk notes (does not like zero, fix this later)
  
b_nroot_min = 60-24 # min pitch
b_nroot_max = 60+12 # max pitch

b_vroot_min = 32    # min velocity
b_vroot_max = 127   # max velocity

b_gnum      = 2     # number of attempted on/offs per beat
b_gprb      = 0.01  # probability of on/off

b_fnum      = 3     # number of attempted pitch changes per beat
b_fprb      = 0.333 # probability of pitch change

b_beta      = 1.25  # inverse temperature

# sequence parameters
 
s_num       = 8     # number of sequences

s_gnum      = 1     # number of attempted on/offs per step
s_gprb      = 0.02  # probability of on/off

s_imin      = 2     # min number of notes in sequence
s_imax      = 6     # max number of notes in sequence

s_nrange    = 12    # max range of pitch
s_nroot_min = 60-24 # min lowest pitch
s_nroot_max = 60+12 # max lowest pitch

s_vrange    = 0     # max range of velocity
s_vroot_min = 64    # min lowest velocity
s_vroot_max = 127   # max lowest velocity

s_beta      = 1.25   # inverse temperature

############################################################################################################################################

###
###  the 'energy' of two notes is equal to the LCD of the two frequencies
###
###    e.g., G is 3/2 the frequency of C, hence these notes have energy 2
###          E is 5/4 the frequency of C, hence these notes have energy 4
###

elist = [1., 15., 8., 5., 4., 3., 45., 2., 5., 3., 5., 8.]

def energy(m, n):
    
    return elist[abs(n - m) % 12]

###
###  the boltzmann factor of the sum of pairwise interactions is used as
###    the probability of picking a particular note
###

def factor(m, n, beta):

    e = 0.
    for i in n:
        e += energy(m, i)

    return np.exp(-beta * e)

###
###  given a key, this chooses a chord
###
###    the root is chosen uniformly from the notes in the key
###    the 3rd and 5th are then chosen by energy
###

ionian = [0, 2, 4, 5, 7, 9, 11]

def choose_chord(key, beta):
    
    a = np.random.choice(map(lambda z: z+key, ionian))
    b = np.random.choice([a+3, a+4], p = norm([factor(a+3, [key], beta), factor(a+4, [key], beta)]))
    c = np.random.choice([a+6, a+7, a+8], p = norm([factor(a+6, [key], beta), factor(a+7, [key], beta), factor(a+8, [key], beta)]))

    return map(lambda z: z%12, (a, b, c))

###
###  given a chord, this chooses a pitch to play
###

def choose_pitch(nmin, nmax, chord, beta):
    
    notes = range(nmin, nmax)    
    probs = norm(map(lambda z: factor(z, chord, beta), notes))
    
    return np.random.choice(notes, p = probs)

############################################################################################################################################

if __name__ == '__main__':
    
    # define the key

    key = 0
    
    # determine chord structure
    
    chord = []
    
    while len(chord) < song_measures:
    
        ctuple = choose_chord(key, b_beta)
        length = np.random.choice(range(chord_measures_min, chord_measures_max))
        
        for _ in range(length):  
            chord.append(ctuple)
    
    song_beats = len(chord) * beats_per_measure
    
    print 'length:', song_beats//60, 'min', song_beats%60, 'sec'

    # OPEN FILE
    
    fp  = open('k_.mid', 'wb')
    
    pos = mid.w_file_head(fp, ppqn)
    
    # TRACK ONE: bulk

    total    = mid.w_track_head(fp, beats_per_measure)
    
    t_now    = 0
    t_prev   = 0
    
    on       = [None] * b_num
    pitch    = [None] * b_num
    velocity = [None] * b_num
    
    # set initial notes

    for b in range(b_num):
        
        on[b]       = np.random.choice(range(2))
        pitch[b]    = choose_pitch(b_nroot_min, b_nroot_max, chord[0], b_beta)
        velocity[b] = np.random.choice(range(b_vroot_min, b_vroot_max))
    
        if on[b]:
            
            total += mid.w_note_on(fp, t_now-t_prev, 1, pitch[b], velocity[b])
            t_prev = t_now
            t_now += ppqn//4
    
    t_start = t_now
    
    # loop through beats
    
    for beat in range(song_beats):
    
        for b in np.random.choice(range(b_num), size=b_fnum):
            
            if np.random.random_sample() < b_fprb:
                
                if on[b]:
                    total += mid.w_note_off(fp, t_now-t_prev, 1, pitch[b], velocity[b])
                
                pitch[b]    = choose_pitch(b_nroot_min, b_nroot_max, chord[beat//beats_per_measure], b_beta)
                velocity[b] = np.random.choice(range(b_vroot_min, b_vroot_max))
                
                if on[b]:
                    total += mid.w_note_on(fp, t_now-t_prev, 1, pitch[b], velocity[b])
    
        for b in np.random.choice(range(b_num), size=b_gnum):
    
            if np.random.random_sample() < b_gprb:
            
                if not on[b]:
                    
                    on[b]  = 1
                    total += mid.w_note_on(fp, t_now-t_prev, 1, pitch[b], velocity[b])
                    t_prev = t_now
                    
                else:
                    
                    on[b]  = 0
                    total += mid.w_note_off(fp, t_now-t_prev, 1, pitch[b], velocity[b])
                    t_prev = t_now
                    
        t_now += ppqn
                    
    # turn shit off
        
    for b in range(b_num):
            
        if on[b]:
            
            on[b]  = 0
            total += mid.w_note_off(fp, t_now-t_prev, 1, pitch[b], velocity[b])
            t_prev = t_now
      
    total += mid.w_track_end(fp)
    
    mid.w_track_fix(fp, pos, total)
    
    pos += 8+total
    
    # TRACK TWO: sequences

    total    = mid.w_track_head(fp, beats_per_measure)
    
    t_now    = t_start
    t_prev   = 0
    
    on       = [None] * s_num
    ind      = [None] * s_num
    nxt      = [None] * s_num
    skip     = [None] * s_num
    pitch    = [None] * s_num
    velocity = [None] * s_num
    
    # define sequences
    
    for s in range(s_num):

        n = np.random.choice(range(s_imin, s_imax))
        r = np.random.choice(range(s_nroot_min, s_nroot_max))
        
        p = [None] * n
        v = [None] * n
    
        for i in range(n):
            
            p[i] = choose_pitch(r, r+s_nrange, [key], s_beta)
            v[i] = np.random.choice(range(s_vroot_min, s_vroot_max))
    
        on[s]       = np.random.choice(range(2))
        ind[s]      = 0
        nxt[s]      = t_now
        skip[s]     = np.random.choice([ppqn//3, ppqn//2, ppqn, ppqn*2, ppqn*3])
        pitch[s]    = p
        velocity[s] = v
        
        if on[s]:
            
            total += mid.w_note_on(fp, t_now-t_prev, s%16, pitch[s][ind[s]], velocity[s][ind[s]])
            ind[s] = (ind[s]+1)%len(pitch[s])
            nxt[s] = t_now + skip[s]
            t_prev = t_now

    # loop over divs
    
    for div in range(song_beats*ppqn):
    
        if on[s]:
            
            if t_now == nxt[s]:
                
                total += mid.w_note_off(fp, t_now-t_prev, s%16, pitch[s][ind[s]], velocity[s][ind[s]])
                t_prev = t_now
        
        if div % ppqn == 0:
        
            for s in np.random.choice(range(s_num), size=s_gnum):

                if np.random.random_sample() < s_gprb:

                    if on[s]:

                        on[s] = 0

                    else:

                        on[s]  = 1
                        ind[s] = 0
                        nxt[s] = t_now

        if on[s]:
            
            if t_now == nxt[s]:

                total += mid.w_note_on(fp, t_now-t_prev, s%16, pitch[s][ind[s]], velocity[s][ind[s]])
                ind[s] = (ind[s]+1)%len(pitch[s])
                nxt[s] = t_now + skip[s]
                t_prev = t_now
            
        t_now += 1

    # finish em
    
    for s in range(s_num):
        
        if on[s]:
            
            total += mid.w_note_off(fp, t_now-t_prev, s%12, pitch[s][ind[s]], velocity[s][ind[s]])
            t_prev = t_now

    total += mid.w_track_end(fp)

    mid.w_track_fix(fp, pos, total)
    
    pos += 8+total

    # CLOSE FILE
    
    fp.close()
            
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
            
            
            
