addpath("~/Desktop/newvision/trunk/mlab/util/");
pvpfile = "/Users/JEC/Desktop/newvision/sandbox/soundAnalysis/output/a9_FullRecon.pvp";
outfilename = "recongraph.png";

[data, hdr] = readpvpfile(pvpfile);


outimg = zeros(hdr.nbands, hdr.nx);


size(squeeze(data{16}.values))

for(time = 1:length(data))

    outimg(time, :) = squeeze(data{time}.values(:,1,1))';

end


outimgrescaled = (outimg - min(outimg(:))) / (max(outimg(:)) - min(outimg(:)));

imwrite(outimgrescaled, outfilename);