
% Filter parameters
hp1 = 100;
hp2 = 3000;
Fs = 53710;
% Fs = 15797;
order = 4;

% Generate and visualize SOS filter
[A, B, C, D] = butter(order/2,[hp1 hp2]/(Fs/2));
sos = ss2sos(A,B,C,D);
freqz(sos,[],Fs)

% Put coefficients in form necessary for CMSIS-DSP "arm_biquad_casc_df1"
sos_cmsisdsp_coeffs = sos(:, [1 2 3 5 6]) .* [1 1 1 -1 -1];

% Output filter coefficients as C array
numStages = size(sos_cmsisdsp_coeffs, 1);

fprintf('const float32_t biquad_coeffs[%d * %d] = {    // %dth order DF1 SOS bandpass, %dHz to %d Hz, fs=%d Hz\n', ...
    5, numStages, order, hp1, hp2, Fs);
for stage = 1:numStages
    fprintf('    // Stage %d\n', stage);
    row = sos_cmsisdsp_coeffs(stage, :);
    fprintf('    %.15g, %.15g, %.15g, %.15g, %.15g', row(1), row(2), row(3), row(4), row(5));
    if stage < numStages
        fprintf(',\n');
    else
        fprintf('\n');
    end
end
fprintf('};\n');