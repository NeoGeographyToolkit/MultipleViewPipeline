I1 = imread('1112_1113-DRG2.tif');
I2 = imread('1113_1114-DRG2.tif');
I3 = imread('1114_1115-DRG2.tif');

n = 64
R1 = [122,448,n,n];
R2 = [270,493,n,n];
R3 = [425,530,n,n];

figure(1)
subplot(2,2,1); imshow(I1(:,:,1)); rectangle('Position',R1,'LineWidth',2,'EdgeColor','r','LineStyle','--');
subplot(2,2,2); imshow(I2(:,:,1)); rectangle('Position',R2,'LineWidth',2,'EdgeColor','g','LineStyle','--');
subplot(2,2,3); imshow(I3(:,:,1)); rectangle('Position',R3,'LineWidth',2,'EdgeColor','b','LineStyle','--');

P1 = I1(R1(2):R1(2)+R1(4)-1,R1(1):R1(1)+R1(3)-1);
P2 = I2(R2(2):R2(2)+R2(4)-1,R2(1):R2(1)+R2(3)-1);
P3 = I3(R3(2):R3(2)+R3(4)-1,R3(1):R3(1)+R2(3)-1);

figure(2)
subplot(2,2,1); imshow(P1);
subplot(2,2,2); imshow(P2);
subplot(2,2,3); imshow(P3);

p1=P1(:);
p2=P2(:);
p3=P3(:);
P = double([P1(:) P2(:) P3(:)]);
[ag, bg, cg] = GaussPhoto(P);
[ap, bp, cp] = PoissPhoto(P);

Ag = uint8(reshape(ag,n,n));

figure(3)
Ap = uint8(reshape(ap,n,n));
subplot(2,2,1); imshow((P1-bg(1))/cg(1));
subplot(2,2,2); imshow((P2-bg(2))/cg(2));
subplot(2,2,3); imshow((P3-bg(3))/cg(3));
subplot(2,2,4); imshow(Ag);

figure(4)
Ap = uint8(reshape(ap,n,n));
subplot(2,2,1); imshow((P1-bp(1))/cp(1));
subplot(2,2,2); imshow((P2-bp(2))/cp(2));
subplot(2,2,3); imshow((P3-bp(3))/cp(3));
subplot(2,2,4); imshow(Ap);

