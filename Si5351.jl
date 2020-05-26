module Si5351

const fxtal = 25e6

function divider_ratios(fout)
    # start with 600 MHz VCO test frequency
    fvco = 600e6
    # integer output divider
    r = 1
    # round up the needed output divider ratio to next highest int
    omd = ceil(Integer, fvco/(fout*r))
    # check wether the VCO frequency for this combination of
    # output divider and output frequency is still valid
    if (fvco < 600e6) || (fvco > 900e6) 
        return nothing
    else
        # calculate PLL feedback ratio
        fmd = omd*fout/fxtal  
        # integer part
        a = floor(Integer, fmd)
        # fractional part
        c = (1<<20)-1
        b = round(Integer, (fmd - a) * c)
        # output divider
        d = omd
        e = 0
        f = 1
        return (a,b,c,d,e,f)
    end
end

function parameters(a,b,c)
    P1 = 128*a + floor(Integer, 128*b/c) - 512
    P2 = 128*b - c*floor(Integer, 128*b/c)
    P3 = c
    (P1, P2, P3)
end

function eiwomisau(fout)
    (a,b,c,d,e,f) = divider_ratios(fout)
    (fb_P1, fb_P2, fb_P3) = parameters(a,b,c)
    (o_P1, o_P2, o_P3) = parameters(d,e,f)
    (fb_P1, fb_P2, fb_P3, o_P1, o_P2, o_P3)
end

end
