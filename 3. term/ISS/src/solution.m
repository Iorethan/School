iptsetpref('UseIPPL',false);
I = double(imread('xvales03.bmp'));
r = size(I,1);
c = size(I,2);
I2 = zeros(r, c);
H = [-0.5 -0.5 -0.5; -0.5 5.0 -0.5; -0.5 -0.5 -0.5];
for x = 1:r
    for y = 1:c
        for a = 1:3
            for b = 1:3
                if((x - 2 + a ~= 0) &&  (x - 2 + a ~= r + 1) && (y - 2 + b ~= 0) && (y - 2 + b ~= c + 1))
                    I2(x,y) = I2(x,y) + H(a,b) * (I(x - 2 + a,y - 2 + b));
                end;
            end;
        end;
    end;
end;
I2 = uint8(I2);
imwrite(uint8(I2),'step1.bmp');

I3 = zeros(r, c);
for x = 1:r
    for y = 1:c
        I3(x,y) = I2(x,c-y+1);
    end;
end;
I3 = uint8(I3);
imwrite(uint8(I3),'step2.bmp');

I4 = zeros(r, c);
I4 = medfilt2(I3, [5 5]);
imwrite(uint8(I4),'step3.bmp');

I5 = zeros(r, c);
H = [1/49 1/49 1/49 1/49 1/49; 1/49 3/49 3/49 3/49 1/49; 1/49 3/49 9/49 3/49 1/49; 1/49 3/49 3/49 3/49 1/49; 1/49 1/49 1/49 1/49 1/49];
%for x = 1:r
%    for y = 1:c
%        for a = 1:5
%            for b = 1:5
%                if((x - 3 + a > 0) && (x - 3 + a < r + 1) && (y - 3 + b > 0) && (y - 3 + b < c + 1))
%                    I5(x,y) = I5(x,y) + H(a,b) * (I4(x - 3 + a,y - 3 + b));
%                end;
%            end;
%        end;
%    end;
%end;
I5 = imfilter(I4, H);
I5 = uint8(I5);
imwrite(uint8(I5),'step4.bmp');

I6 = zeros(r, c);
for x = 1:r
    for y = 1:c
        I6(x,y) = I5(x,c-y+1);
    end;
end;


chyba = 0;
for x = 1:r
    for y = 1:c
        chyba = chyba + abs(I(x,y) - I6(x,y));
    end;
end;
chyba = chyba / (r*c)

mn = min(reshape(im2double(I5),[r*c,1]));
mx = max(reshape(im2double(I5),[r*c,1]));

I7 = zeros(r, c);
%for x = 1:r
%    for y = 1:c
%        I7(x,y) = (I5(x,y)-mn) * step;
%    end;
%end;
I7 = imadjust(I5, [mn; mx], [0.0; 1.0]);
I7 = im2uint8(I7);
imwrite(I7,'step5.bmp');

mean_no_hist = mean(mean(I5))
std_no_hist= std(reshape(double(I5),[r*c,1]))
mean_hist= mean(mean(I7))
std_hist= std(reshape(double(I7),[r*c,1]))

I8 = zeros(r, c);
N = 2;
a = 0;
b = 255;
for x = 1:r
    for y = 1:c
        I8(x,y) = round(((2^N)-1)*(double(I7(x,y))-a)/(b-a))*(b-a)/((2^N)-1) + a;
    end;
end;
I8 = uint8(I8);
imwrite(uint8(I8),'step6.bmp');

%{
Ixt = imread('ref/step1.bmp');
subplot(121); imshow(I2); subplot(122); imshow(Ixt);
sum(sum(abs(double(I2) - double(Ixt))))

Ixt = imread('ref/step2.bmp');
subplot(121); imshow(I3); subplot(122); imshow(Ixt);
sum(sum(abs(double(I3) - double(Ixt))))

Ixt = imread('ref/step3.bmp');
subplot(121); imshow(I4); subplot(122); imshow(Ixt);
sum(sum(abs(double(I4) - double(Ixt))))

Ixt = imread('ref/step4.bmp');
subplot(121); imshow(I5); subplot(122); imshow(Ixt);
sum(sum(abs(double(I5) - double(Ixt))))

Ixt = imread('ref/step5.bmp');
subplot(121); imshow(I7); subplot(122); imshow(Ixt);
sum(sum(abs(double(I7) - double(Ixt))))

Ixt = imread('ref/step6.bmp');
subplot(121); imshow(I8); subplot(122); imshow(Ixt);
sum(sum(abs(double(I8) - double(Ixt))))
%}
