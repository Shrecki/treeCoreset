function [TemporalMask] = AssessMotionFullArgs(Path,soi,Folder_motion,TA_mot_prefix, skipped_scans_motionfile, skipped_scans, FD_method, FD_threshold, filename)
    fid = fopen(filename, 'w');
    % compatibility with input as cell of strings for every subject or for a
    % simple string with the same subfoldeer for all subjects
    if ~isempty(Folder_motion) && iscell(Folder_motion)
        if length(Folder_motion)~=n_subjects
            error('Folder_motion: wrong number of motion subfolders');
        end
        Folder_motion=Folder_motion{soi};
    end
    
    if ~isempty(TA_mot_prefix) && iscell(TA_mot_prefix)
        if length(TA_mot_prefix)~=n_subjects
            error('TA_mot_prefix: wrong number of motion prefix names');
        end
        TA_mot_prefix=TA_mot_prefix{soi};
    end
    
    % Checks that the input folder and the folders containing the data
    % exist
    if ~exist(fullfile(Path,Folder_motion),'dir')
        error('There is no motion folder...');
    end
    
    if isempty(skipped_scans_motionfile)
        skipped_scans_motionfile=skipped_scans;
    end
    
    % Reads the motion file
    tmp_data = cellstr(spm_select('List',fullfile(Path,Folder_motion),['^' TA_mot_prefix '.*\.' 'txt' '$']));
    
    if length(tmp_data) ~= 1
        error('More than 1 motion file (or no motion file)...');
    else
        RP = textread(fullfile(Path,Folder_motion,tmp_data{1}), '',  'delimiter', '  ');
    end

    % Removes the first scans
    RP=RP(skipped_scans_motionfile+1:end,:);
    disp(RP(1,7))
    
    disp(tmp_data);

    % Converts the rotational displacement values into [mm] (are given
    % as [rad] by SPM)
    RP(:,4:6)=RP(:,4:6)*180/pi;

    switch FD_method
        case 'Power'
            
            %Calculate FD Power (Power, J.D., Barnes, K.A., Snyder, A.Z., 
            % Schlaggar, B.L., Petersen, S.E., 2012. Spurious but systematic 
            % correlations in functional connectivity MRI networks arise from 
            % subject motion. Neuroimage 59, 2142-2154.) 
            
            % Frame-to-frame change in motion
            RPDiff=diff(RP);
            disp(size(RPDiff))
            RPDiff=[zeros(1,6);RPDiff];
            
            % Converts into [mm] displacement around a 50mm radius sphere
            RPDiffSphere=RPDiff;
            RPDiffSphere(:,4:6)=RPDiffSphere(:,4:6)*50*pi/180; % displacement on surface of sphere with radius 50 mm, arc length = theta(rad) * r
            
            % Computes FD_Power itself as the sum of absolute valued
            % elements
            FD_Power=sum(abs(RPDiffSphere),2);
            
            % Average FD_Power
            MeanFD_Power = mean(FD_Power);

            PercentFD_Power = length(find(FD_Power>FD_threshold)) / length(FD_Power) * 100;
           
            TemporalMask=ones(size(RP,1),1);
            TemporalMask(FD_Power > FD_threshold) = 0;
            TemporalMask = logical(TemporalMask);
           
            if ~isempty(fid)
                fprintf(fid,['Average motion:  ',num2str(MeanFD_Power),' [mm]']);
                fprintf(fid,['Motion: there are ',num2str(PercentFD_Power),'percent of data scrubbed']);
            end
        otherwise
            error('Scrubbing method not recognized - only FD Power is implemented so far for scubbing...');      

    end

end